#include <rpp/rpp.h>

#include <iostream>

/**
 * \example map.cpp
 **/

int main()
{
    //! [Same type]
    rpp::source::create<int>([](const auto& sub)
            {
                sub.on_next(42);
            })
            .map([](int value)
            {
                return value + 10;
            })
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 52
    //! [Same type]

    //! [Changed type]
    rpp::source::create<int>([](const auto& sub)
            {
                sub.on_next(42);
            })
            .map([](int value)
            {
                return std::to_string(value) + " VAL";
            })
            .subscribe([](std::string v) { std::cout << v << std::endl; });
    // Output: 42 VAL
    //! [Changed type]
    return 0;
}
