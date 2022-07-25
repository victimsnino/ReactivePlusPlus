#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example distinct_until_changed.cpp
 **/

int main()
{
    //! [distinct_until_changed]
    rpp::source::just(1, 1, 2, 2, 3, 2, 1)
            .distinct_until_changed()
            .subscribe([](int val) { std::cout << val << " "; });
    // Output: 1 2 3 2 1
    //! [distinct_until_changed]
    return 0;
}
