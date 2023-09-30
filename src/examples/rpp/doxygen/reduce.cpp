#include <rpp/rpp.hpp>

#include <iostream>
#include <string>

/**
 * @example reduce.cpp
 **/

int main()
{
    //! [reduce]
    rpp::source::just(1, 2, 3)
        | rpp::operators::reduce(std::string{}, [](std::string&& seed, int v) {return std::move(seed) + std::to_string(v) + ","; })
        | rpp::operators::subscribe([](const std::string& v) { std::cout << v << std::endl; });
    // Output: 1,2,3,
    //! [reduce]

    //! [reduce_no_seed]
    rpp::source::just(1, 2, 3)
        | rpp::operators::reduce(std::plus<int>{})
        | rpp::operators::subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 6
    //! [reduce_no_seed]
    return 0;
}
