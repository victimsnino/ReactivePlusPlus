#include <rpp/rpp.hpp>
#include <iostream>

/**
 * @example multicast.cpp
 **/
int main() // NOLINT(bugprone-exception-escape)
{
    {
        //! [multicast]
        auto subject = rpp::subjects::publish_subject<int>{};
        auto observable = rpp::source::just(1, 2, 3) | rpp::operators::multicast(subject);
        observable.subscribe([](int v) {std::cout << "#1 " << v << std::endl; });
        observable.subscribe([](int v) {std::cout << "#2 " << v << std::endl; });
        observable.connect();
        // Output:
        // #1 1
        // #2 1
        // #1 2
        // #2 2
        // #1 3
        // #2 3
        //! [multicast]
    }
    {
        //! [multicast_template]
        auto observable = rpp::source::just(1, 2, 3) | rpp::operators::multicast<rpp::subjects::publish_subject>();
        observable.subscribe([](int v) {std::cout << "#1 " << v << std::endl; });
        observable.subscribe([](int v) {std::cout << "#2 " << v << std::endl; });
        observable.connect();
        // Output:
        // #1 1
        // #2 1
        // #1 2
        // #2 2
        // #1 3
        // #2 3
        //! [multicast_template]
    }
    {
        //! [publish]
        auto observable = rpp::source::just(1, 2, 3) | rpp::operators::publish();
        observable.subscribe([](int v) {std::cout << "#1 " << v << std::endl; });
        observable.subscribe([](int v) {std::cout << "#2 " << v << std::endl; });
        observable.connect();
        // Output:
        // #1 1
        // #2 1
        // #1 2
        // #2 2
        // #1 3
        // #2 3
        //! [publish]
    }
    return 0;
}
