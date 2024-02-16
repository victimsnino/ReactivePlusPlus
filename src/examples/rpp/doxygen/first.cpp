#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example first.cpp
 **/
int main()
{
    //! [first]
    rpp::source::just(1, 2, 3, 4, 5)
        | rpp::operators::first()
        | rpp::operators::subscribe(
            [](const auto& v) { std::cout << "-" << v; },
            [](const std::exception_ptr&) {},
            []() { std::cout << "-|" << std::endl; });
    // Source: -1-2-3-4-5--|
    // Output: -1-|
    //! [first]

    //! [first_empty]
    rpp::source::empty<int>()
        | rpp::operators::first()
        | rpp::operators::subscribe(
            [](const auto& v) { std::cout << "-" << v; },
            [](const std::exception_ptr&) { std::cout << "-x" << std::endl; },
            []() { std::cout << "-|" << std::endl; });
    // Source: -1-2-3-4-5--|
    // Output: -x
    //! [first_empty]
    return 0;
}