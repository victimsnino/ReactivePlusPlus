#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example skip.cpp
 **/
int main()
{
    //! [skip]
    rpp::source::just(0, 1, 2, 3, 4, 5)
        | rpp::operators::skip(2)
        | rpp::operators::subscribe([](int v) { std::cout << v << " "; });
    // Output: 2 3 4 5
    //! [skip]
    return 0;
}