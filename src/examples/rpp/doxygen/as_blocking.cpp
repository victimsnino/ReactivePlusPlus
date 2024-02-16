#include <rpp/rpp.hpp>

#include "rpp/sources/fwd.hpp"

#include <iostream>

/**
 * \example as_blocking.cpp
 **/

int main() // NOLINT(bugprone-exception-escape)
{
    //! [as_blocking]
    rpp::source::just(1)
        | rpp::operators::delay(std::chrono::seconds{1}, rpp::schedulers::new_thread{}) // <-- emit from another thread with delay
        | rpp::operators::as_blocking()
        | rpp::operators::subscribe([](int) {}, []() { std::cout << "COMPLETED" << std::endl; });
    std::cout << "done" << std::endl;
    // output: COMPLETED done
    //! [as_blocking]
}