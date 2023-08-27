#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example defer.cpp
 **/

int main() // NOLINT
{
    //! [defer from_iterable]
    rpp::source::defer([] { return rpp::source::from_iterable(std::vector<int>{ 1,2,3 }); }).subscribe([](int v) {std::cout << v << " "; });
    // Output: 1 2 3
    //! [defer from_iterable]
}