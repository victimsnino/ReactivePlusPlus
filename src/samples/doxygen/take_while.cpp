#include <rpp/rpp.h>
#include <iostream>

/**
 * \example take_while.cpp
 **/
int main()
{
    //! [take_while]
    rpp::source::create<int>([](const auto& sub)
            {
                for (int i = 0; i < 10; ++i)
                    sub.on_next(i);
            })
            .take_while([](int v) { return v != 5; })
            .subscribe([](int  v) { std::cout << v << " "; });
    // Output: 0 1 2 3 4
    //! [take_while]
    return 0;
}
