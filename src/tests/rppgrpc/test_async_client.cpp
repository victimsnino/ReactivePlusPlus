#include <catch2/catch_test_macros.hpp>

#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/operators/observe_on.hpp>
#include <rpp/schedulers/new_thread.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include <grpc++/server_builder.h>
#include <rppgrpc/proto.grpc.pb.h>
#include <rppgrpc/proto.pb.h>
#include <rppgrpc/rppgrpc.hpp>

#include "rpp_trompeloil.hpp"

#include <future>

namespace
{
    struct service : public trompeloeil::mock_interface<TestService::Service>
    {
        IMPLEMENT_MOCK3(ServerSide);
        IMPLEMENT_MOCK3(ClientSide);
        IMPLEMENT_MOCK2(Bidirectional);
    };
} // namespace

TEST_CASE("async client reactor")
{
    grpc::ServerBuilder   builder{};
    trompeloeil::sequence s{};

    auto mock_service = std::make_unique<service>();

    builder.RegisterService(mock_service.get());

    auto       server(builder.BuildAndStart());
    const auto channel = server->InProcessChannel({});

    const auto stub = TestService::NewStub(channel, {});

    rpp::subjects::publish_subject<uint32_t> subj{};
    mock_observer<uint32_t>                  out_mock{};

    SECTION("bidirectional")
    {
        grpc::ClientContext ctx{};

        const auto bidi_reactor = new rppgrpc::client_bidi_reactor<Request, Response>();
        bidi_reactor->get_observable() | rpp::ops::map([](const Response& out) { return out.value(); })  | rpp::ops::subscribe(out_mock);
        subj.get_observable() | rpp::ops::map([](int v) { Request request{}; request.set_value(v); return request; }) | rpp::ops::subscribe(bidi_reactor->get_observer());

        stub->async()->Bidirectional(&ctx, bidi_reactor);
        SECTION("no stream job - completion")
        {
            REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_)).RETURN(grpc::Status::OK).IN_SEQUENCE(s);
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            bidi_reactor->init();

            wait(last);
        }

        SECTION("error status - error")
        {
            REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_)).RETURN(grpc::Status::CANCELLED).IN_SEQUENCE(s);
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);
            bidi_reactor->init();

            wait(last);
        }

        SECTION("manual server-side cancel")
        {
            REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_))
                .RETURN(grpc::Status::OK)
                .LR_SIDE_EFFECT({
                    ctx.TryCancel();
                });

            bidi_reactor->init();

            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);

            wait(last);
        }

        SECTION("manual client-side completion")
        {
            REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_))
                .RETURN(grpc::Status::OK)
                .LR_SIDE_EFFECT({
                    Request request{};
                    while (_2->Read(&request))
                    {
                    }
                });

            bidi_reactor->init();

            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            subj.get_observer().on_completed();

            wait(last);
        }

        SECTION("client-side write + completion")
        {
            std::promise<std::vector<int>> results{};
            REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_))
                .RETURN(grpc::Status::OK)
                .LR_SIDE_EFFECT({
                    std::vector<int> reads{};
                    Request          request{};
                    while (_2->Read(&request))
                    {
                        reads.push_back(request.value());
                    }
                    results.set_value(reads);
                });


            subj.get_observer().on_next(1);
            subj.get_observer().on_next(2);

            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            subj.get_observer().on_completed();

            bidi_reactor->init();

            auto f = results.get_future();
            REQUIRE(f.wait_for(std::chrono::seconds{1}) == std::future_status::ready);
            CHECK(f.get() == std::vector<int>{1, 2});

            wait(last);
        }

        SECTION("client-side read + completion")
        {
            REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_))
                .RETURN(grpc::Status::OK)
                .LR_SIDE_EFFECT({
                    Response response{};

                    for (int v : {1, 2, 3})
                    {
                        response.set_value(v);
                        _2->Write(response);
                    }
                });
            bidi_reactor->init();

            REQUIRE_CALL(*out_mock, on_next_rvalue(1)).IN_SEQUENCE(s);
            REQUIRE_CALL(*out_mock, on_next_rvalue(2)).IN_SEQUENCE(s);
            REQUIRE_CALL(*out_mock, on_next_rvalue(3)).IN_SEQUENCE(s);
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);

            wait(last);
        }

        SECTION("client-side read-write + completeion")
        {
            REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_))
                .RETURN(grpc::Status::OK)
                .LR_SIDE_EFFECT({
                    Request request{};
                    while (_2->Read(&request))
                    {
                        Response response{};
                        response.set_value(request.value() * 10);
                        _2->Write(response);
                    }
                });


            REQUIRE_CALL(*out_mock, on_next_rvalue(10)).IN_SEQUENCE(s);
            subj.get_observer().on_next(1);

            REQUIRE_CALL(*out_mock, on_next_rvalue(20)).IN_SEQUENCE(s);
            subj.get_observer().on_next(2);

            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            subj.get_observer().on_completed();

            bidi_reactor->init();

            wait(last);
        }
    }
    SECTION("server-side")
    {
        grpc::ClientContext ctx{};

        const auto read_reactor = new rppgrpc::client_read_reactor<Response>();
        read_reactor->get_observable() | rpp::ops::map([](const Response& out) { return out.value(); }) | rpp::ops::subscribe(out_mock);

        SECTION("empty request")
        {
            stub->async()->ServerSide(&ctx, nullptr, read_reactor);

            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);
            read_reactor->init();

            wait(last);
        }
        SECTION("normal request")
        {
            Request r{};
            stub->async()->ServerSide(&ctx, &r, read_reactor);

            SECTION("no stream job - completion")
            {
                REQUIRE_CALL(*mock_service, ServerSide(trompeloeil::_, trompeloeil::_, trompeloeil::_)).RETURN(grpc::Status::OK).IN_SEQUENCE(s);
                const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
                read_reactor->init();

                wait(last);
            }

            SECTION("error status - error")
            {
                REQUIRE_CALL(*mock_service, ServerSide(trompeloeil::_, trompeloeil::_, trompeloeil::_)).RETURN(grpc::Status::CANCELLED).IN_SEQUENCE(s);
                const auto last = NAMED_REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);
                read_reactor->init();

                wait(last);
            }

            SECTION("manual server-side cancel")
            {
                REQUIRE_CALL(*mock_service, ServerSide(trompeloeil::_, trompeloeil::_, trompeloeil::_))
                    .RETURN(grpc::Status::OK)
                    .LR_SIDE_EFFECT({
                        ctx.TryCancel();
                    });

                read_reactor->init();

                const auto last = NAMED_REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);

                wait(last);
            }

            SECTION("client-side read + completion")
            {
                REQUIRE_CALL(*mock_service, ServerSide(trompeloeil::_, trompeloeil::_, trompeloeil::_))
                    .RETURN(grpc::Status::OK)
                    .LR_SIDE_EFFECT({
                        Response response{};

                        for (int v : {1, 2, 3})
                        {
                            response.set_value(v);
                            _3->Write(response);
                        }
                    });
                read_reactor->init();

                REQUIRE_CALL(*out_mock, on_next_rvalue(1)).IN_SEQUENCE(s);
                REQUIRE_CALL(*out_mock, on_next_rvalue(2)).IN_SEQUENCE(s);
                REQUIRE_CALL(*out_mock, on_next_rvalue(3)).IN_SEQUENCE(s);
                const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);

                wait(last);
            }
        }
    }
    SECTION("client-side")
    {
        grpc::ClientContext ctx{};

        const auto write_reactor = new rppgrpc::client_write_reactor<Request>();
        write_reactor->get_observable() | rpp::ops::map([](const rpp::utils::none& out) { return 0; })  | rpp::ops::subscribe(out_mock);
        subj.get_observable() | rpp::ops::map([](int v) { Request request{}; request.set_value(v); return request; }) | rpp::ops::subscribe(write_reactor->get_observer());

        Response r{};
        stub->async()->ClientSide(&ctx, &r, write_reactor);
        SECTION("no stream job - completion")
        {
            REQUIRE_CALL(*mock_service, ClientSide(trompeloeil::_, trompeloeil::_, trompeloeil::_)).RETURN(grpc::Status::OK).IN_SEQUENCE(s);
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            write_reactor->init();

            wait(last);
        }

        SECTION("error status - error")
        {
            REQUIRE_CALL(*mock_service, ClientSide(trompeloeil::_, trompeloeil::_, trompeloeil::_)).RETURN(grpc::Status::CANCELLED).IN_SEQUENCE(s);
            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);
            write_reactor->init();

            wait(last);
        }

        SECTION("manual server-side cancel")
        {
            REQUIRE_CALL(*mock_service, ClientSide(trompeloeil::_, trompeloeil::_, trompeloeil::_))
                .RETURN(grpc::Status::OK)
                .LR_SIDE_EFFECT({
                    ctx.TryCancel();
                });

            write_reactor->init();

            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);

            wait(last);
        }

        SECTION("manual client-side completion")
        {
            REQUIRE_CALL(*mock_service, ClientSide(trompeloeil::_, trompeloeil::_, trompeloeil::_))
                .RETURN(grpc::Status::OK)
                .LR_SIDE_EFFECT({
                    Request request{};
                    while (_2->Read(&request))
                    {
                    }
                });

            write_reactor->init();

            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            subj.get_observer().on_completed();

            wait(last);
        }

        SECTION("client-side write + completion")
        {
            std::promise<std::vector<int>> results{};
            REQUIRE_CALL(*mock_service, ClientSide(trompeloeil::_, trompeloeil::_, trompeloeil::_))
                .RETURN(grpc::Status::OK)
                .LR_SIDE_EFFECT({
                    std::vector<int> reads{};
                    Request          request{};
                    while (_2->Read(&request))
                    {
                        reads.push_back(request.value());
                    }
                    results.set_value(reads);
                });


            subj.get_observer().on_next(1);
            subj.get_observer().on_next(2);

            const auto last = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            subj.get_observer().on_completed();

            write_reactor->init();

            auto f = results.get_future();
            REQUIRE(f.wait_for(std::chrono::seconds{1}) == std::future_status::ready);
            CHECK(f.get() == std::vector<int>{1, 2});

            wait(last);
        }
    }
    server->Shutdown();
    server->Wait();
}
