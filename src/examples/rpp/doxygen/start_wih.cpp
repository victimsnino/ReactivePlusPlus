#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example start_with.cpp
 **/
int main()
{
    //! [start_with_values]
    rpp::source::just(1, 2, 3)
        | rpp::ops::start_with(5, 6)
        | rpp::ops::subscribe([](int v) { std::cout << v << " "; });
    // Output: 5 6 1 2 3
    //! [start_with_values]

    //! [start_with_observable]
    rpp::source::just(1, 2, 3)
        | rpp::ops::start_with(rpp::source::just(5), rpp::source::just(6))
        | rpp::ops::subscribe([](int v) { std::cout << v << " "; });
    // Output: 5 6 1 2 3
    //! [start_with_observable]

    //! [start_with_observable_as_value]
    rpp::source::just(rpp::source::just(1))
        | rpp::ops::start_with_values(rpp::source::just(5), rpp::source::just(6))
        | rpp::ops::merge()
        | rpp::ops::subscribe([](int v) { std::cout << v << " "; });
    // Output: 5 6 1
    //! [start_with_observable_as_value]

    return 0;
}