#include <rpp/rpp.hpp>
#include <iostream>

/**
 * @example ref_count.cpp
 **/
int main() // NOLINT(bugprone-exception-escape)
{
    //! [ref_count]
    auto observable = rpp::source::just(1, 2, 3) | rpp::operators::multicast();
    observable.subscribe([](int v) {std::cout << "#1 " << v << std::endl; });
    // No Output

    observable.ref_count().subscribe([](int v) {std::cout << "#2 " << v << std::endl; });
    // Output:
    // #1 1
    // #2 1
    // #1 2
    // #2 2
    // #1 3
    // #2 3

    //! [ref_count]
    return 0;
}
