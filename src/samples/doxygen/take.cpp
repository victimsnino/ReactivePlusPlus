#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example take.cpp
 **/
int main()
{
    //! [take]
    rpp::source::create<int>([](const auto& sub)
            {
                for (int i = 0; i < 10; ++i)
                    sub.on_next(i);
            })
            .take(2)
            .subscribe([](int v) { std::cout << v << " "; });
    // Output: 0 1
    //! [take]
    return 0;
}
