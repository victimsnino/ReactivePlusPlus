#include <rpp/rpp.hpp>

#include "rpp/sources/fwd.hpp"

#include <iostream>

/**
 * \example connect.cpp
 **/

int main() // NOLINT(bugprone-exception-escape)
{
    //! [connect]
    const auto observable = rpp::source::interval(std::chrono::milliseconds{50}, rpp::schedulers::new_thread{})
                          | rpp::ops::map([](int v) {
                                std::cout << "value in map" << v << std::endl;
                                return v;
                            })
                          | rpp::ops::publish();

    std::cout << "CONNECT" << std::endl;
    auto d = observable.connect(); // subscribe happens right now

    std::this_thread::sleep_for(std::chrono::milliseconds{150});

    std::cout << "SUBSCRIBE" << std::endl;
    observable.subscribe([](int v) { std::cout << "observer value " << v << std::endl; });

    std::this_thread::sleep_for(std::chrono::milliseconds{150});

    d.dispose();
    std::cout << "DISPOSE" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds{150});

    // possible output:
    // CONNECT
    // value in map0
    // value in map1
    // value in map2
    // SUBSCRIBE
    // value in map3
    // observer value 3
    // value in map4
    // observer value 4
    // value in map5
    // observer value 5
    // DISPOSE
    //! [connect]
}