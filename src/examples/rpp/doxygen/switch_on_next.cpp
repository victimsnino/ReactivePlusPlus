#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example switch_on_next.cpp
 **/
int main()
{
    //! [switch_on_next]
    rpp::source::just(rpp::source::just(1).as_dynamic(),
                      rpp::source::never<int>().as_dynamic(),
                      rpp::source::just(2).as_dynamic())
            | rpp::operators::switch_on_next()
            | rpp::operators::subscribe([](int v) { std::cout << v << " "; });
    // Output: 1 2
    //! [switch_on_next]

    return 0;
}