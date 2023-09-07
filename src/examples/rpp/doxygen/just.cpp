#include <rpp/rpp.hpp>

#include <iostream>
#include <array>

/**
 * \example just.cpp
 **/

int main() // NOLINT(bugprone-exception-escape)
{
    //! [just]
    rpp::source::just(42, 53, 10, 1).subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 42 53 10 1
    //! [just]

    //! [just memory model]
    std::array<int, 100> expensive_to_copy_1{};
    std::array<int, 100> expensive_to_copy_2{};
    rpp::source::just<rpp::memory_model::use_shared>(expensive_to_copy_1, expensive_to_copy_2).subscribe([](const auto&){});
    //! [just memory model]

    // //! [just scheduler]
    rpp::source::just(rpp::schedulers::immediate{}, 42, 53).subscribe([](const auto&){});
    rpp::source::just(rpp::schedulers::current_thread{}, 42, 53).subscribe([](const auto&){});
    // //! [just scheduler]
}
