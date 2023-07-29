#include <iostream>
#include <rpp/rpp.hpp>


/**
 * \example map.cpp
 **/

int main() // NOLINT
{
rpp::source::just('1', 'a', 'W', '2', '0', 'f', 'q')
                    | rpp::operators::repeat()
                    | rpp::operators::take_while([](char v) { return v != '0'; })
                    | rpp::operators::filter(std::not_fn(&::isdigit))
                    | rpp::operators::map(&::toupper)
                    | rpp::operators::subscribe([](char v) { std::cout << v; });
    return 0;
}