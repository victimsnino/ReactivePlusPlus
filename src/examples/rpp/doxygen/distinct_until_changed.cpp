#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example distinct_until_changed.cpp
 **/

int main()
{
    //! [distinct_until_changed]
    rpp::source::just(1, 1, 2, 2, 3, 2, 1)
            | rpp::operators::distinct_until_changed()
            | rpp::operators::subscribe([](int val) { std::cout << val << " "; });
    // Output: 1 2 3 2 1
    //! [distinct_until_changed]

    std::cout << std::endl;
    //! [distinct_until_changed_with_comparator]
    rpp::source::just(1, 1, 2, 2, 3, 2, 1)
            | rpp::operators::distinct_until_changed([](int left, int right) {return left != right; })
            | rpp::operators::subscribe([](int val) { std::cout << val << " "; });
    // Output: 1 1 1
    //! [distinct_until_changed_with_comparator]
    return 0;
}
