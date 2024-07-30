#include <catch2/catch_test_macros.hpp>

#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/operators/observe_on.hpp>
#include <rpp/schedulers/new_thread.hpp>

#include <grpc++/server_builder.h>
#include <rppgrpc/proto.grpc.pb.h>
#include <rppgrpc/proto.pb.h>
#include <rppgrpc/rppgrpc.hpp>

#include "rpp_trompeloil.hpp"

struct service : public trompeloeil::mock_interface<TestService::CallbackService>
{
    MAKE_MOCK2(ServerSide, (grpc::ServerWriteReactor<::Response>*)(::grpc::CallbackServerContext* /*context*/, const ::Request* /*request*/));
    MAKE_MOCK2(ClientSide, (::grpc::ServerReadReactor<::Request>*)(::grpc::CallbackServerContext* /*context*/, ::Response* /*response*/));
    MAKE_MOCK1(Bidirectional, (::grpc::ServerBidiReactor<::Request, ::Response>*)(::grpc::CallbackServerContext* /*context*/));
};

void wait(const std::unique_ptr<trompeloeil::expectation>& e)
{
    while (!e->is_satisfied())
    {
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }
}


TEST_CASE("Async server")
{
    grpc::ServerBuilder   builder{};
    trompeloeil::sequence s{};

    auto mock_service = std::make_unique<service>();

    builder.RegisterService(mock_service.get());

    auto server(builder.BuildAndStart());

    const auto channel = server->InProcessChannel({});
    const auto stub    = TestService::NewStub(channel, {});

    mock_observer<uint32_t> out_mock{};

    SECTION("bidirectionl")
    {
        grpc::ClientContext ctx{};
        const auto          reactor = new rppgrpc::server_bidi_reactor<Request, Response>();
        reactor->get_observable() | rpp::ops::map([](const Request& out) { return out.value(); }) | rpp::ops::observe_on(rpp::schedulers::new_thread{}) | rpp::ops::subscribe(out_mock);

        ::grpc::CallbackServerContext* obtained_context{};
        const auto                     bidirectional_call = NAMED_REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_)).LR_SIDE_EFFECT(obtained_context = _1;).RETURN(reactor).IN_SEQUENCE(s);

        const auto writer = stub->Bidirectional(&ctx);

        wait(bidirectional_call);

        SECTION("writer immediate finish")
        {
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            auto       t    = std::thread{[&] {
                writer->WritesDone();
                CHECK(writer->Finish().ok());
            }};

            reactor->get_observer().on_completed();
            t.join();
            wait(last);
        }

        SECTION("writer cancels")
        {
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);
            ctx.TryCancel();
            wait(last);
        }

        SECTION("server cancels")
        {
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);
            obtained_context->TryCancel();
            wait(last);
        }

        SECTION("writer writes")
        {

            REQUIRE_CALL(*out_mock, on_next_rvalue(1)).IN_SEQUENCE(s);
            REQUIRE_CALL(*out_mock, on_next_rvalue(2)).IN_SEQUENCE(s);
            REQUIRE_CALL(*out_mock, on_next_rvalue(3)).IN_SEQUENCE(s);
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            std::thread{[&] {
                Request request{};
                for (int i : {1, 2, 3})
                {
                    request.set_value(i);
                    REQUIRE(writer->Write(request));
                }
                writer->WritesDone();
            }}.join();

            reactor->get_observer().on_completed();

            std::thread{[&] {
                REQUIRE(writer->Finish().ok());
            }}.join();

            wait(last);
        }

        SECTION("writer reads")
        {
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            Response   response{};
            for (int i : {1, 2, 3})
            {
                response.set_value(i);
                reactor->get_observer().on_next(response);
            }
            reactor->get_observer().on_completed();

            std::thread{[&] {
                writer->WritesDone();

                Response response{};
                for (int i : {1, 2, 3})
                {
                    REQUIRE(writer->Read(&response));
                    REQUIRE(response.value() == i);
                }
                REQUIRE(!writer->Read(&response));
                REQUIRE(writer->Finish().ok());
            }}.join();
            wait(last);
        }
    }

    server->Shutdown();
    server->Wait();
}
