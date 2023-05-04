#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example take.cpp
 **/
int main() // NOLINT
{
    //! [take]
    rpp::source::from_iterable(std::vector{0,1,2,3,4})
            | rpp::operators::take(2)
            | rpp::operators::subscribe([](int v) { std::cout << v << " "; });
    // Output: 0 1
    //! [take]
    return 0;
}