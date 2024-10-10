#include <rpp/rpp.hpp>

#include <functional>
#include <iostream>

/**
 * @example readme.cpp 
 */

int main() // NOLINT(bugprone-exception-escape)
{
    // ![readme]
    rpp::source::from_callable(&::getchar)
        | rpp::operators::repeat()
        | rpp::operators::take_while([](char v) { return v != '0'; })
        | rpp::operators::filter(std::not_fn(&::isdigit))
        | rpp::operators::map(&::toupper)
        | rpp::operators::subscribe([](char v) { std::cout << v; });
    // ![readme]

    return 0;
}
