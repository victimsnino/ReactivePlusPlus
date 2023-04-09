#include <rpp/rpp.hpp>

#include <iostream>
#include <array>

/**
 * \example just.cpp
 **/

int main() // NOLINT
{
    //! [just]
    rpp::source::just(42, 53, 10, 1).subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 42 53 10 1
    //! [just]

    //! [just memory model]
    std::array<int, 100> cheap_to_copy_1{};
    std::array<int, 100> cheap_to_copy_2{};
    rpp::source::just<rpp::memory_model::use_shared>(cheap_to_copy_1, cheap_to_copy_2).subscribe([](const auto&){});
    //! [just memory model]

    // //! [just scheduler]
    // rpp::source::just(rpp::schedulers::new_thread{}, 42, 53).subscribe();
    // //! [just scheduler]
}