#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example skip.cpp
 **/
int main()
{
    //! [skip]
    rpp::source::create<int>([](const auto& sub)
            {
                for (int i = 0; i < 5; ++i)
                    sub.on_next(i);
            })
            .skip(2)
            .subscribe([](int v) { std::cout << v << " "; });
    // Output: 2 3 4
    //! [skip]
    return 0;
}
