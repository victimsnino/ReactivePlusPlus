#include <rpp/rpp.hpp>

#include <grpc++/create_channel.h>
#include <rppgrpc/rppgrpc.hpp>

#include "protocol.grpc.pb.h"
#include "protocol.pb.h"


int main()
{
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    if (!channel)
    {
        std::cout << "NO CHANNEL" << std::endl;
        return 0;
    }
    auto stub = TestService::NewStub(channel);
    if (!stub)
    {
        std::cout << "NO STUB" << std::endl;
        return 0;
    }

    std::array<grpc::ClientContext, 3> ctx{};
    auto                               d = rpp::composite_disposable_wrapper::make();

    rpp::subjects::publish_subject<std::string> bidi_requests{};
    rpp::subjects::publish_subject<Response>    bidi_responses{};
    bidi_responses.get_observable().subscribe(d, [](const Response& v) {
        std::cout << "[BidireactionalResponse]: " << v.ShortDebugString() << std::endl;
    });

    rppgrpc::add_client_reactor(&TestService::StubInterface::async_interface::Bidirectional,
                                *stub->async(),
                                &ctx[0],
                                bidi_requests.get_observable()
                                    | rpp::ops::take_while([](const std::string& v) { return v != "0"; })
                                    | rpp::ops::map([](const std::string& v) {
                                          Request i{};
                                          i.set_value(std::string{"BidiRequest "} + v);
                                          return i;
                                      }),
                                bidi_responses.get_observer());

    rppgrpc::add_client_reactor(&TestService::StubInterface::async_interface::ClientSide,
                                *stub->async(),
                                &ctx[1],
                                bidi_responses.get_observable()
                                    | rpp::ops::map([](const Response& response) {
                                          Request request{};
                                          request.set_value(std::string{"ClientSideRequest "} + response.value());
                                          return request;
                                      }),
                                rpp::make_lambda_observer(d, [](const Response& v) {
                                    std::cout << "[ClientsideResponse]: " << v.ShortDebugString() << std::endl;
                                }));
    Request req{};
    rppgrpc::add_client_reactor(&TestService::StubInterface::async_interface::ServerSide,
                                *stub->async(),
                                &ctx[2],
                                &req,
                                rpp::make_lambda_observer(d, [](const Response& v) {
                                    std::cout << "[ServerSideResponse]: " << v.ShortDebugString() << std::endl;
                                }));

    std::cout << "SUBSCRIBED" << std::endl;

    std::string in{};
    while (!d.is_disposed())
    {
        std::getline(std::cin, in);
        bidi_requests.get_observer().on_next(in);
        in.clear();
    }

    return 0;
}
