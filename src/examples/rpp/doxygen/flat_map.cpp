#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example flat_map.cpp
 **/
int main()
{
    //! [flat_map]
    rpp::source::just(1, 2, 3)
            .flat_map([](int val) { return rpp::source::from_iterable(std::vector(val, val)); })
            .subscribe([](int v) { std::cout << v << " "; });
    // Output: 1 2 2 3 3 3
    //! [flat_map]
    return 0;
}
