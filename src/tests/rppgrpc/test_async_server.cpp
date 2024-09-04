#include <catch2/catch_test_macros.hpp>

#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/operators/observe_on.hpp>

#include <grpc++/server_builder.h>
#include <rppgrpc/proto.grpc.pb.h>
#include <rppgrpc/proto.pb.h>
#include <rppgrpc/rppgrpc.hpp>

#include "rpp_trompeloil.hpp"


namespace
{
    struct service : public trompeloeil::mock_interface<TestService::CallbackService>
    {
        using write_reactor_ptr = grpc::ServerWriteReactor<Response>*;
        using read_reactor_ptr  = grpc::ServerReadReactor<Request>*;
        using bidi_reactor_ptr  = grpc::ServerBidiReactor<Request, Response>*;

        MAKE_MOCK2(ServerSide, write_reactor_ptr(grpc::CallbackServerContext* /*context*/, const Request* /*request*/));
        MAKE_MOCK2(ClientSide, read_reactor_ptr(grpc::CallbackServerContext* /*context*/, Response* /*response*/));
        MAKE_MOCK1(Bidirectional, bidi_reactor_ptr(grpc::CallbackServerContext* /*context*/));
    };
} // namespace


TEST_CASE("Async server")
{
    grpc::ServerBuilder   builder{};
    trompeloeil::sequence s{};

    auto mock_service = std::make_unique<service>();

    builder.RegisterService(mock_service.get());

    auto server(builder.BuildAndStart());

    const auto channel = server->InProcessChannel({});
    const auto stub    = TestService::NewStub(channel, {});

    mock_observer<uint32_t>      out_mock{};
    grpc::ClientContext          ctx{};
    grpc::CallbackServerContext* obtained_context{};

    auto test_common = [&](const auto& writer, const auto* reactor) {
        SECTION("writer immediate finish")
        {
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            if constexpr (requires { writer->WritesDone(); })
                writer->WritesDone();

            if constexpr (requires { reactor->get_observer(); })
                reactor->get_observer().on_completed();

            CHECK(writer->Finish().ok());
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
    };

    auto test_read = [&](const auto& writer, const auto* reactor) {
        SECTION("writer writes")
        {
            REQUIRE_CALL(*out_mock, on_next_rvalue(1)).IN_SEQUENCE(s);
            REQUIRE_CALL(*out_mock, on_next_rvalue(2)).IN_SEQUENCE(s);
            REQUIRE_CALL(*out_mock, on_next_rvalue(3)).IN_SEQUENCE(s);
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            Request    request{};
            for (int i : {1, 2, 3})
            {
                request.set_value(i);
                REQUIRE(writer->Write(request));
            }
            writer->WritesDone();

            if constexpr (requires { reactor->get_observer(); })
                reactor->get_observer().on_completed();

            REQUIRE(writer->Finish().ok());

            wait(last);
        }
    };

    auto test_write = [&](const auto& writer, const auto* reactor) {
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

            if constexpr (requires { writer->WritesDone(); })
                writer->WritesDone();

            for (int i : {1, 2, 3})
            {
                REQUIRE(writer->Read(&response));
                REQUIRE(response.value() == i);
            }
            REQUIRE(!writer->Read(&response));
            REQUIRE(writer->Finish().ok());
            wait(last);
        }
    };

    SECTION("bidirectionl")
    {
        const auto reactor = new rppgrpc::server_bidi_reactor<Request, Response>();
        reactor->get_observable() | rpp::ops::map([](const Request& out) { return out.value(); }) | rpp::ops::subscribe(out_mock);

        const auto call = NAMED_REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_)).LR_SIDE_EFFECT(obtained_context = _1;).RETURN(reactor).IN_SEQUENCE(s);

        const auto writer = stub->Bidirectional(&ctx);

        wait(call);

        test_common(writer, reactor);
        test_read(writer, reactor);
        test_write(writer, reactor);
    }

    SECTION("server-side")
    {
        const auto reactor = new rppgrpc::server_write_reactor<Response>();
        reactor->get_observable() | rpp::ops::map([](const rpp::utils::none&) { return 0; })  | rpp::ops::subscribe(out_mock);

        const auto call = NAMED_REQUIRE_CALL(*mock_service, ServerSide(trompeloeil::_, trompeloeil::_)).LR_SIDE_EFFECT(obtained_context = _1;).RETURN(reactor).IN_SEQUENCE(s);

        const auto writer = stub->ServerSide(&ctx, {});

        wait(call);

        test_common(writer, reactor);
        test_write(writer, reactor);
    }

    SECTION("client-side")
    {
        const auto reactor = new rppgrpc::server_read_reactor<Request>();
        reactor->get_observable() | rpp::ops::map([](const Request& out) { return out.value(); }) | rpp::ops::subscribe(out_mock);

        const auto call = NAMED_REQUIRE_CALL(*mock_service, ClientSide(trompeloeil::_, trompeloeil::_)).LR_SIDE_EFFECT(obtained_context = _1;).RETURN(reactor).IN_SEQUENCE(s);

        Response   response{};
        const auto writer = stub->ClientSide(&ctx, &response);

        wait(call);

        test_common(writer, reactor);
        test_read(writer, reactor);
    }

    server->Shutdown();
    server->Wait();
}
