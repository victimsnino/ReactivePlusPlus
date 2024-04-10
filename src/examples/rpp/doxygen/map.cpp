#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example map.cpp
 **/

int main() // NOLINT(bugprone-exception-escape)
{
    //! [Same type]
    rpp::source::just(42)
        | rpp::operators::map([](int value) { return value + 10; })
        | rpp::operators::subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 52
    //! [Same type]

    //! [Changed type]
    rpp::source::just(42)
        | rpp::operators::map([](int value) { return std::to_string(value) + " VAL"; })
        | rpp::operators::subscribe([](const std::string& v) { std::cout << v << std::endl; });
    // Output: 42 VAL
    //! [Changed type]
    return 0;
}
