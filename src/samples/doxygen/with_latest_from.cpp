#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example with_latest_from.cpp
 **/
int main()
{
    //! [with_latest_from]
    rpp::source::just(1, 2, 3).with_latest_from(rpp::source::just(3, 4, 5, 6))
        .subscribe([](std::tuple<int,int> v) { std::cout << std::get<0>(v) << ":" << std::get<1>(v) << " "; });
    // Output: 1:6 2:6 3:6
    //! [with_latest_from]

    //! [with_latest_from custom selector]
    rpp::source::just(1, 2, 3).with_latest_from([](int left, int right) { return left + right; },
                                                rpp::source::just(3, 4, 5, 6))
                              .subscribe([](int v)
                              {
                                  std::cout << v << " ";
                              });
    // Output: 7 8 9
    //! [with_latest_from custom selector]
    return 0;
}
