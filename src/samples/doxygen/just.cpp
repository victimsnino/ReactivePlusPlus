#include <rpp/rpp.h>

#include <iostream>
#include <array>

/**
 * \example just.cpp
 **/

int main()
{
    //! [just]
    rpp::source::just(42).subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 42
    //! [just]

    //! [just memory model]
    std::array<int, 100> cheap_to_copy{};
    rpp::source::just<rpp::memory_model::use_shared>(cheap_to_copy).subscribe();
    //! [just memory model]

    //! [just scheduler]
    rpp::source::just(42, rpp::schedulers::new_thread{}).subscribe();
    //! [just scheduler]
}
