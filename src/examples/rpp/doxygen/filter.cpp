#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example filter.cpp
 **/
int main() // NOLINT(bugprone-exception-escape)
{
    //! [Filter]
    rpp::source::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
        | rpp::operators::filter([](int v) { return v % 2 == 0; })
        | rpp::operators::subscribe([](int v) { std::cout << v << " "; });
    // Output: 0 2 4 6 8
    //! [Filter]
    return 0;
}