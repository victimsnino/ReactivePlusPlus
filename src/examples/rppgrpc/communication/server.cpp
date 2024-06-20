
#include <rpp/operators/map.hpp>

#include <grpc++/server_builder.h>
#include <rppgrpc/rppgrpc.hpp>

#include "protocol.grpc.pb.h"
#include "protocol.pb.h"

class Service : public TestService::CallbackService
{
public:
    Service()
    {
        client_side_requests.get_observable().subscribe([](const Request& s) { std::cout << "[ClientSideRequest]: " << s.ShortDebugString() << std::endl; });
    }

    grpc::ServerBidiReactor<::Request, ::Response>* Bidirectional(::grpc::CallbackServerContext* /*context*/) override
    {
        rpp::subjects::publish_subject<Response> response{};
        rpp::subjects::publish_subject<Request>  request{};
        request.get_observable()
            | rpp::ops::subscribe([](const Request& s) { std::cout << "[BidireactionalRequest]: " << s.ShortDebugString() << std::endl; });
        request.get_observable()
            | rpp::ops::map([](const Request& request) {
                  Response response{};
                  response.set_value(std::string{"BidiResponse "} + request.value());
                  return response;
              })
            | rpp::ops::subscribe(response.get_observer());
        return rppgrpc::make_server_reactor(response.get_observable(), request.get_observer());
    }

    ::grpc::ServerReadReactor<::Request>* ClientSide(::grpc::CallbackServerContext* /*context*/, ::Response* /*response*/) override
    {
        return rppgrpc::make_server_reactor(client_side_requests.get_observer());
    }

    ::grpc::ServerWriteReactor<::Response>* ServerSide(::grpc::CallbackServerContext* /*context*/, const ::Request* /*request*/) override
    {
        return rppgrpc::make_server_reactor(client_side_requests.get_observable()
                                            | rpp::ops::map([](const Request& v) {
                                                  Response response{};
                                                  response.set_value(std::string{"ServerSideResponse "} + v.value());
                                                  return response;
                                              }));
    }

private:
    rpp::subjects::publish_subject<Request> client_side_requests{};
};

int main()
{
    Service             service{};
    grpc::ServerBuilder builder{};

    std::string server_address("localhost:50051");

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    auto server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();

    return 0;
}
