#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example filter.cpp
 **/
int main()
{
    //! [Filter]
    rpp::source::create<int>([](const auto& sub)
            {
                for (int i = 0; i < 10; ++i)
                    sub.on_next(i);
            })
            .filter([](int    v) { return v % 2 == 0; })
            .subscribe([](int v) { std::cout << v << " "; });
    // Output: 0 2 4 6 8
    //! [Filter]
    return 0;
}
