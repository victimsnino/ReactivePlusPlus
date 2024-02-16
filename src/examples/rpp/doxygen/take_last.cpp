#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example take_last.cpp
 **/
int main()
{
    //! [take_last]
    rpp::source::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
        | rpp::ops::take_last(2)
        | rpp::ops::subscribe([](int v) { std::cout << v << " "; });
    // Output: 8 9
    //! [take_last]
    return 0;
}
