#include <rpp/rpp.hpp>

#include <cctype>
#include <chrono>
#include <iostream>

int main() // NOLINT
{
    auto source = rpp::source::concat(rpp::source::just('1', 'w', 'e', '2', 'r','3')
        | rpp::operators::repeat(3), rpp::source::just('P', '0'))
        | rpp::operators::take_while([](char v) { return v != '0'; });

    auto chars = source
        | rpp::operators::filter(std::not_fn(&::isdigit))
        | rpp::operators::map([](char c) -> char {return static_cast<char>(std::toupper(c));});

    auto digits = source
        | rpp::operators::filter([](char c) -> bool {return std::isdigit(c); });

    chars
        | rpp::operators::merge_with(digits)
        | rpp::operators::skip(3)
        | rpp::operators::distinct_until_changed()
        | rpp::operators::subscribe([](char v) { std::cout << v; });


    return 0;
}