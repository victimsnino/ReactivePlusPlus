#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example multicast.cpp
 **/
int main()
{
    //! [multicast]
    auto subject = rpp::subjects::publish_subject<int>{};
    auto observable = rpp::source::just(1, 2, 3).multicast(subject);
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
    return 0;
}
