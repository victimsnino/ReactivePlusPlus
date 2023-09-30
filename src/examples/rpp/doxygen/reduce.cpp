#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example reduce.cpp
 **/

int main()
{
    //! [reduce]
    rpp::source::just(1, 2, 3)
        | rpp::operators::reduce(10, std::plus<int>{})
        | rpp::operators::subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 16
    //! [reduce]

    //! [reduce_no_seed]
    rpp::source::just(1, 2, 3)
        | rpp::operators::reduce(std::plus<int>{})
        | rpp::operators::subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 6
    //! [reduce_no_seed]
    return 0;
}
