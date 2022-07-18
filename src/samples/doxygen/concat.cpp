#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example concat.cpp
 **/
int main()
{
    //! [concat]
    rpp::source::just(rpp::source::just(1).as_dynamic(),
                      rpp::source::never<int>().as_dynamic(),
                      rpp::source::just(2).as_dynamic())
            .concat()
            .subscribe([](int v) { std::cout << v << " "; });
    // Output: 1
    //! [concat]

    //! [concat_with]
    rpp::source::just(1)
            .concat_with(rpp::source::just(2), rpp::source::never<int>(), rpp::source::just(3))
            .subscribe([](int v) { std::cout << v << " "; });
    // Output: 1 2
    //! [concat_with]
    return 0;
}
