#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example with_latest_from.cpp
 **/
int main()
{
    //! [with_latest_from]
    rpp::source::just(1, 2, 3, 4, 5, 6)
        | rpp::operators::with_latest_from(rpp::source::just(3, 4, 5))
        | rpp::operators::subscribe([](std::tuple<int, int> v) { std::cout << std::get<0>(v) << ":" << std::get<1>(v) << " "; });
    // Output: 1:3 2:4 3:5 4:5 5:5 6:5

    std::cout << std::endl;

    rpp::source::just(1, 2, 3)
        | rpp::operators::with_latest_from(rpp::source::just(3, 4, 5, 6, 7, 8))
        | rpp::operators::subscribe([](std::tuple<int, int> v) { std::cout << std::get<0>(v) << ":" << std::get<1>(v) << " "; });
    // Output: 1:3 2:4 3:5
    //! [with_latest_from]

    std::cout << std::endl;

    //! [with_latest_from custom selector]
    rpp::source::just(1, 2, 3, 4)
        | rpp::operators::with_latest_from([](int left, int right) { return left + right; },
                                           rpp::source::just(3, 4, 5))
        | rpp::operators::subscribe([](int v) { std::cout << v << " "; });
    // Output: 4 6 8 9
    //! [with_latest_from custom selector]
    return 0;
}
