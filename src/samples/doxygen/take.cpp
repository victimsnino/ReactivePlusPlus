#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example take.cpp
 **/
int main()
{
    //! [take]
    rpp::source::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
            .take(2)
            .subscribe([](int v) { std::cout << v << " "; });
    // Output: 0 1
    //! [take]
    return 0;
}
