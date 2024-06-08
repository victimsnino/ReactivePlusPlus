#include <rpp/rpp.hpp>

#include <grpc++/create_channel.h>
#include <rppgrpc/rppgrpc.hpp>

#include "protocol.grpc.pb.h"
/**
 * \example client_reactor.cpp
 **/

int main() // NOLINT(bugprone-exception-escape)
{
    {
        //! [bidi_reactor]
        auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
        auto stub    = TestService::NewStub(channel);

        grpc::ClientContext                      ctx{};
        rpp::subjects::publish_subject<Request>  requests{};
        rpp::subjects::publish_subject<Response> responses{};
        rppgrpc::add_client_reactor(&TestService::StubInterface::async_interface::Bidirectional, *stub->async(), &ctx, requests.get_observable(), responses.get_observer());
        //! [bidi_reactor]
    }
    {
        //! [read_reactor]
        auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
        auto stub    = TestService::NewStub(channel);

        grpc::ClientContext                      ctx{};
        rpp::subjects::publish_subject<Response> responses{};
        Request                                  request{};
        rppgrpc::add_client_reactor(&TestService::StubInterface::async_interface::ServerSide, *stub->async(), &ctx, &request, responses.get_observer());
        //! [read_reactor]
    }
    {
        //! [write_reactor]
        auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
        auto stub    = TestService::NewStub(channel);

        grpc::ClientContext                      ctx{};
        rpp::subjects::publish_subject<Request>  requests{};
        rpp::subjects::publish_subject<Response> responses{};
        rppgrpc::add_client_reactor(&TestService::StubInterface::async_interface::ClientSide, *stub->async(), &ctx, requests.get_observable(), responses.get_observer());
        //! [write_reactor]
    }

    return 0;
}
