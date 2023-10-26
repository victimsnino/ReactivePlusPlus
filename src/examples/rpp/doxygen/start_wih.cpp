#include <rpp/rpp.hpp>
#include <iostream>

/**
 * @example start_with.cpp
 **/
int main()
{
    //! [start_with_values]
    rpp::source::just(1,2,3)
        | rpp::ops::start_with_values(5, 6)
        | rpp::ops::subscribe([](int v) { std::cout << v << " "; });
    // Output: 5 6 1 2 3
    //! [start_with_values]

    //! [start_with_observable]
    rpp::source::just(1, 2, 3)
        | rpp::ops::start_with(rpp::source::just(5), rpp::source::just(6))
        | rpp::ops::subscribe([](int v) { std::cout << v << " "; });
    // Output: 5 6 1 2 3
    //! [start_with_observable]

    return 0;
}