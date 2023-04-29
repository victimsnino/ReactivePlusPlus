#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example take_while.cpp
 **/
int main() // NOLINT
{
    //! [take_while]
    rpp::source::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
            | rpp::operators::take_while([](int v) { return v != 5; })
            | rpp::operators::subscribe([](int  v) { std::cout << v << " "; });
    // Output: 0 1 2 3 4
    //! [take_while]
    return 0;
}