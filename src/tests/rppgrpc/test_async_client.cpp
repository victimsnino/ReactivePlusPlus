#include <catch2/catch_test_macros.hpp>

#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include <rppgrpc/proto.grpc.pb.h>
#include <rppgrpc/proto.pb.h>
#include <rppgrpc/rppgrpc.hpp>

#include "rpp_trompeloil.hpp"

struct async_interface : public trompeloeil::mock_interface<TestService::StubInterface::async_interface>
{
    IMPLEMENT_MOCK3(ServerSide);
    IMPLEMENT_MOCK3(ClientSide);
    IMPLEMENT_MOCK2(Bidirectional);
};

struct ClientCallbackReaderWriter : public trompeloeil::mock_interface<grpc::ClientCallbackReaderWriter<Request, Response>>
{
    using grpc::ClientCallbackReaderWriter<Request, Response>::BindReactor;

    IMPLEMENT_MOCK0(StartCall);
    IMPLEMENT_MOCK2(Write);
    IMPLEMENT_MOCK0(WritesDone);
    IMPLEMENT_MOCK1(Read);
    IMPLEMENT_MOCK1(AddHold);
    IMPLEMENT_MOCK0(RemoveHold);
};

struct ClientCallbackReader : public trompeloeil::mock_interface<grpc::ClientCallbackReader<Response>>
{
    using grpc::ClientCallbackReader<Response>::BindReactor;

    IMPLEMENT_MOCK0(StartCall);
    IMPLEMENT_MOCK1(Read);
    IMPLEMENT_MOCK1(AddHold);
    IMPLEMENT_MOCK0(RemoveHold);
};

struct ClientCallbackWriter : public trompeloeil::mock_interface<grpc::ClientCallbackWriter<Request>>
{
    using grpc::ClientCallbackWriter<Request>::BindReactor;

    IMPLEMENT_MOCK0(StartCall);
    MAKE_MOCK2(Write, void(const Request* req, grpc::WriteOptions options), override);
    IMPLEMENT_MOCK0(WritesDone);
    IMPLEMENT_MOCK1(AddHold);
    IMPLEMENT_MOCK0(RemoveHold);
};

TEST_CASE("async client can be casted to rppgrpc")
{
    grpc::ClientContext ctx{};
    Response*           resp{};

    rpp::subjects::publish_subject<Request>  subj{};
    rpp::subjects::publish_subject<Response> out_subj{};

    async_interface                  stub_mock{};
    mock_observer_strategy<uint32_t> mock{};
    out_subj.get_observable() | rpp::ops::map([](const Response& out) { return out.value(); }) | rpp::ops::subscribe(mock);


    auto validate_write = [&](auto& stream_mock, auto*& reactor) {
        SECTION("write to stream")
        {
            for (auto v : {10, 3, 15, 20})
            {
                Request request{};
                request.set_value(v);

                REQUIRE_CALL(stream_mock, Write(trompeloeil::_, trompeloeil::_)).WITH(_1->value() == v);

                subj.get_observer().on_next(request);
                reactor->OnWriteDone(true);
            }
        }
        SECTION("write failed")
        {
            CHECK(mock.get_on_error_count() == 0);
            reactor->OnWriteDone(false);
            reactor = nullptr;
            CHECK(mock.get_on_error_count() == 1);
        }
    };

    auto validate_read = [&](auto& stream_mock, auto*& reactor) {
        SECTION("read from stream")
        {
            std::vector<uint32_t> expected_values{};
            for (auto v : {3, 5, 2, 1})
            {
                resp->set_value(v);
                CHECK(mock.get_received_values() == expected_values);

                REQUIRE_CALL(stream_mock, Read(trompeloeil::_));
                reactor->OnReadDone(true);

                expected_values.push_back(v);
                CHECK(mock.get_received_values() == expected_values);
            }
        }
        SECTION("read failed")
        {
            CHECK(mock.get_on_error_count() == 0);
            reactor->OnReadDone(false);
            reactor = nullptr;
            CHECK(mock.get_on_error_count() == 1);
        }
    };

    SECTION("bidirectional")
    {
        grpc::ClientBidiReactor<::Request, ::Response>* reactor{};

        ClientCallbackReaderWriter stream_mock{};
        trompeloeil::sequence      stream_sequence{};

        REQUIRE_CALL(stub_mock, Bidirectional(trompeloeil::_, trompeloeil::_)).LR_SIDE_EFFECT({
            reactor = _2;
            CHECK(_1 == &ctx);
            stream_mock.BindReactor(reactor);
        });

        const auto temp_reactor = new rppgrpc::client_bidi_reactor<Request, Response>();
        stub_mock.Bidirectional(&ctx, temp_reactor);
        temp_reactor->get_observable().subscribe(out_subj.get_observer());
        subj.get_observable().subscribe(temp_reactor->get_observer());

        REQUIRE_CALL(stream_mock, StartCall()).IN_SEQUENCE(stream_sequence);
        REQUIRE_CALL(stream_mock, Read(trompeloeil::_)).LR_SIDE_EFFECT({ resp = _1; }).IN_SEQUENCE(stream_sequence);

        temp_reactor->init();

        validate_write(stream_mock, reactor);
        validate_read(stream_mock, reactor);

        if (reactor)
        {
            reactor->OnDone(grpc::Status::OK);
        }
    }
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
}
