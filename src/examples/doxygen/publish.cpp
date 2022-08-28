#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example publish.cpp
 **/
int main()
{
    //! [publish]
    auto observable = rpp::source::just(1,2,3).publish();
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
    return 0;
}
