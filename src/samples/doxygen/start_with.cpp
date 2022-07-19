#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example start_with.cpp
 **/
int main()
{
    //! [start_with]
    rpp::source::just(1,2,3)
        .start_with(5, 6)
        .subscribe([](int v) { std::cout << v << " "; });
    // Output: 5 6 1 2 3
    //! [start_with]

    //! [start_with observable]
    rpp::source::just(1, 2, 3)
        .start_with(rpp::source::just(5), rpp::source::just(6))
        .subscribe([](int v) { std::cout << v << " "; });
    // Output: 5 6 1 2 3
    //! [start_with observable]

    return 0;
}
