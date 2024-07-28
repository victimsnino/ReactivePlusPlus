#include <catch2/catch_test_macros.hpp>

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

TEST_CASE("Async server")
{
    grpc::ServerBuilder builder{};

    auto mock_service = std::make_unique<service>();

    builder.RegisterService(mock_service.get());

    auto server(builder.BuildAndStart());

    const auto channel = server->InProcessChannel({});
    const auto stub    = TestService::NewStub(channel, {});

    SECTION("bidirectionl")
    {
        grpc::ClientContext ctx{};
        Request             req{};
        req.set_value(32);
        REQUIRE_CALL(*mock_service, Bidirectional(trompeloeil::_))
            .WITH(_1.value() == 32)
            .RETURN(new rppgrpc::server_bidi_reactor<Response, Request>());

        const auto writer = stub->Bidirectional(&ctx);
    }

    server->Shutdown();
    server->Wait();
}
