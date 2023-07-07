#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example last.cpp
 **/
int main()
{
    //! [last]
    rpp::source::just(1, 2, 3, 4, 5)
        | rpp::operators::last()
        | rpp::operators::subscribe(
            [](const auto &v) { std::cout << "-" << v; },
            [](const std::exception_ptr &) {},
            []() { std::cout << "-|" << std::endl; });
    // Source: -1-2-3-4-5--|
    // Output: -5-|
    //! [last]

    //! [last empty]
    rpp::source::empty<int>()
        | rpp::operators::last()
        | rpp::operators::subscribe(
            [](const auto &v) { std::cout << "-" << v; },
            [](const std::exception_ptr &) { std::cout << "-x"; },
            []() { std::cout << "-|" << std::endl; });
    // Source: -1-2-3-4-5--|
    // Output: -x
    //! [last empty]
    return 0;
}