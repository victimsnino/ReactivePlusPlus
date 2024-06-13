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

struct service : public trompeloeil::mock_interface<TestService::Service>
{
    IMPLEMENT_MOCK3(ServerSide);
    IMPLEMENT_MOCK3(ClientSide);
    IMPLEMENT_MOCK2(Bidirectional);
};

void wait(const std::unique_ptr<trompeloeil::expectation>& e)
{
    const auto start = std::chrono::system_clock::now();
    while (!e->is_satisfied() && std::chrono::system_clock::now() - start < std::chrono::seconds{1})
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

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
        bidi_reactor->get_observable() | rpp::ops::map([](const Response& out) { return out.value(); }) | rpp::ops::observe_on(rpp::schedulers::new_thread{}) | rpp::ops::subscribe(out_mock);
        subj.get_observable() | rpp::ops::map([](int v) { Request request{}; request.set_value(v); return request; }) | rpp::ops::subscribe(bidi_reactor->get_observer());


        stub->async()->Bidirectional(&ctx, bidi_reactor);
        SECTION("no stream job - completion")
        {
            const auto initial_call = NAMED_REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_)).RETURN(grpc::Status::OK).IN_SEQUENCE(s);
            REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            bidi_reactor->init();

            wait(initial_call);
        }
        SECTION("error status - error")
        {
            const auto initial_call = NAMED_REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_)).RETURN(grpc::Status::CANCELLED).IN_SEQUENCE(s);
            REQUIRE_CALL(*out_mock, on_error(trompeloeil::_)).IN_SEQUENCE(s);
            bidi_reactor->init();

            wait(initial_call);
        }
        SECTION("manual client-side completion")
        {
            const auto initial_call = NAMED_REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_))
                                          .RETURN(grpc::Status::OK)
                                          .LR_SIDE_EFFECT({
                                              Request request{};
                                              while (_2->Read(&request))
                                              {
                                              }
                                          });

            bidi_reactor->init();

            const auto completed = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            subj.get_observer().on_completed();

            wait(completed);
        }
        SECTION("client-side write + completion")
        {
            std::promise<std::vector<int>> results{};
            const auto                     initial_call = NAMED_REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_, trompeloeil::_))
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

            bidi_reactor->init();
            subj.get_observer().on_next(1);
            subj.get_observer().on_next(2);

            const auto completed = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            subj.get_observer().on_completed();

            auto f = results.get_future();
            REQUIRE(f.wait_for(std::chrono::seconds{1}) == std::future_status::ready);
            CHECK(f.get() == std::vector<int>{1, 2});

            wait(completed);
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
            const auto completed = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);

            wait(completed);
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

            bidi_reactor->init();

            REQUIRE_CALL(*out_mock, on_next_rvalue(10)).IN_SEQUENCE(s);
            subj.get_observer().on_next(1);

            REQUIRE_CALL(*out_mock, on_next_rvalue(20)).IN_SEQUENCE(s);
            subj.get_observer().on_next(2);

            const auto completed = NAMED_REQUIRE_CALL(*out_mock, on_completed()).IN_SEQUENCE(s);
            subj.get_observer().on_completed();

            wait(completed);
        }
    }

    // auto validate_write = [&](auto& stream_mock, auto*& reactor) {
    //     SECTION("write to stream")
    //     {
    //         for (auto v : {10, 3, 15, 20})
    //         {
    //             Request request{};
    //             request.set_value(v);

    //             REQUIRE_CALL(stream_mock, Write(trompeloeil::_, trompeloeil::_)).WITH(_1->value() == v);

    //             subj.get_observer().on_next(request);
    //             reactor->OnWriteDone(true);
    //         }
    //     }
    //     SECTION("write failed")
    //     {
    //         CHECK(mock.get_on_error_count() == 0);
    //         reactor->OnWriteDone(false);
    //         reactor = nullptr;
    //         CHECK(mock.get_on_error_count() == 1);
    //     }
    // };

    // auto validate_read = [&](auto& stream_mock, auto*& reactor) {
    //     SECTION("read from stream")
    //     {
    //         std::vector<uint32_t> expected_values{};
    //         for (auto v : {3, 5, 2, 1})
    //         {
    //             resp->set_value(v);
    //             CHECK(mock.get_received_values() == expected_values);

    //             REQUIRE_CALL(stream_mock, Read(trompeloeil::_));
    //             reactor->OnReadDone(true);

    //             expected_values.push_back(v);
    //             CHECK(mock.get_received_values() == expected_values);
    //         }
    //     }
    //     SECTION("read failed")
    //     {
    //         CHECK(mock.get_on_error_count() == 0);
    //         reactor->OnReadDone(false);
    //         reactor = nullptr;
    //         CHECK(mock.get_on_error_count() == 1);
    //     }
    // };

    // SECTION("bidirectional")
    // {
    //     grpc::ClientBidiReactor<::Request, ::Response>* reactor{};

    //     ClientCallbackReaderWriter stream_mock{};
    //     trompeloeil::sequence      stream_sequence{};

    //     REQUIRE_CALL(stub_mock, Bidirectional(trompeloeil::_, trompeloeil::_)).LR_WITH(_1 == &ctx).LR_SIDE_EFFECT({
    //         reactor = _2;
    //         stream_mock.BindReactor(reactor);
    //     });

    //     const auto temp_reactor = new rppgrpc::client_bidi_reactor<Request, Response>();
    //     stub_mock.Bidirectional(&ctx, temp_reactor);
    //     temp_reactor->get_observable().subscribe(out_subj.get_observer());
    //     subj.get_observable().subscribe(temp_reactor->get_observer());

    //     REQUIRE_CALL(stream_mock, StartCall()).IN_SEQUENCE(stream_sequence);
    //     REQUIRE_CALL(stream_mock, Read(trompeloeil::_)).LR_SIDE_EFFECT({ resp = _1; }).IN_SEQUENCE(stream_sequence);

    //     temp_reactor->init();

    //     validate_write(stream_mock, reactor);
    //     validate_read(stream_mock, reactor);

    //     if (reactor)
    //     {
    //         reactor->OnDone(grpc::Status::OK);
    //     }
    // }
    // SECTION("server-side")
    // {
    //     grpc::ClientReadReactor<Response>* reactor{};

    //     ClientCallbackReader  stream_mock{};
    //     trompeloeil::sequence stream_sequence{};

    //     REQUIRE_CALL(stream_mock, StartCall()).IN_SEQUENCE(stream_sequence);
    //     REQUIRE_CALL(stream_mock, Read(trompeloeil::_)).LR_SIDE_EFFECT({ resp = _1; }).IN_SEQUENCE(stream_sequence);

    //     Request message{};

    //     REQUIRE_CALL(stub_mock, ServerSide(trompeloeil::_, trompeloeil::_, trompeloeil::_)).LR_SIDE_EFFECT({
    //         reactor = _3;
    //         CHECK(_2 == &message);
    //         CHECK(_1 == &ctx);
    //         stream_mock.BindReactor(reactor);
    //     });

    //     rppgrpc::add_client_reactor(&TestService::StubInterface::async_interface::ServerSide, stub_mock, &ctx, &message, out_subj.get_observer());

    //     validate_read(stream_mock, reactor);

    //     if (reactor)
    //     {
    //         reactor->OnDone(grpc::Status::OK);
    //     }
    // }
    // SECTION("client-side")
    // {
    //     grpc::ClientWriteReactor<::Request>* reactor{};

    //     ClientCallbackWriter  stream_mock{};
    //     trompeloeil::sequence stream_sequence{};

    //     REQUIRE_CALL(stream_mock, StartCall()).IN_SEQUENCE(stream_sequence);

    //     REQUIRE_CALL(stub_mock, ClientSide(trompeloeil::_, trompeloeil::_, trompeloeil::_)).LR_SIDE_EFFECT({
    //         reactor = _3;
    //         resp    = _2;
    //         CHECK(_1 == &ctx);
    //         stream_mock.BindReactor(reactor);
    //     });

    //     rppgrpc::add_client_reactor(&TestService::StubInterface::async_interface::ClientSide, stub_mock, &ctx, subj.get_observable(), out_subj.get_observer());

    //     validate_write(stream_mock, reactor);

    //     SECTION("get response")
    //     {
    //         CHECK(resp);
    //         resp->set_value(30);

    //         REQUIRE_CALL(stream_mock, WritesDone());
    //         subj.get_observer().on_completed();
    //         CHECK(mock.get_total_on_next_count() == 0);
    //         reactor->OnDone(grpc::Status::OK);
    //         reactor = nullptr;

    //         CHECK(mock.get_received_values() == std::vector<uint32_t>{30});
    //     }
    //     if (reactor)
    //     {
    //         reactor->OnDone(grpc::Status::OK);
    //     }
    // }
    server->Shutdown();
    server->Wait();
}
