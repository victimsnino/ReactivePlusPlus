#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example create.cpp
 **/

int main()
{
    //! [create]
    rpp::source::create<int>([](const auto& sub)
            {
                sub.on_next(42);
            })
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 42
    //! [create]

    //! [create type deduction]
    rpp::source::create([](const rpp::dynamic_subscriber<int>& sub)
            {
                sub.on_next(42);
            })
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 42
    //! [create type deduction]

    //! [create with capture]
    int                                        val = 42;
    rpp::source::create<int>([val](const auto& sub)
            {
                sub.on_next(val);
            })
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 42
    //! [create with capture]
}
