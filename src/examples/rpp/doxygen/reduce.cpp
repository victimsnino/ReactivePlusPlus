#include <rpp/rpp.hpp>

#include <iostream>
#include <string>

using namespace std::string_literals;

/**
 * @example reduce.cpp
 **/

int main()
{
    //! [reduce]
    rpp::source::just("b"s, "c"s)
        | rpp::operators::reduce("a"s, std::plus<std::string>{})
        | rpp::operators::subscribe([](std::string v) { std::cout << v << std::endl; });
    // Output: abc
    //! [reduce]

    //! [reduce_no_seed]
    rpp::source::just("a"s, "b"s, "c"s)
        | rpp::operators::reduce(std::plus<std::string>{})
        | rpp::operators::subscribe([](std::string v) { std::cout << v << std::endl; });
    // Output: abc
    //! [reduce_no_seed]
    return 0;
}
