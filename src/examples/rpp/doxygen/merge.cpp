#include <rpp/rpp.hpp>
#include <iostream>

/**
 * @example merge.cpp
 **/
int main()
{
    //! [merge]
    rpp::source::just(rpp::source::just(1).as_dynamic(),
                      rpp::source::never<int>().as_dynamic(),
                      rpp::source::just(2).as_dynamic())
            | rpp::operators::merge()
            | rpp::operators::subscribe([](int v) { std::cout << v << " "; });
    // Output: 1 2
    //! [merge]

    //! [merge_with]
    rpp::source::just(1)
            | rpp::operators::merge_with(rpp::source::just(2))
            | rpp::operators::subscribe([](int v) { std::cout << v << " "; });
    // Output: 1 2
    //! [merge_with]
    return 0;
}
