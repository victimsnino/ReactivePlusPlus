#include <rpp/rpp.h>
#include <iostream>

/**
 * \example merge.cpp
 **/
int main()
{
    //! [merge]
    rpp::source::just(rpp::source::just(1).as_dynamic(),
                      rpp::source::never<int>().as_dynamic(),
                      rpp::source::just(2).as_dynamic())
            .merge()
            .subscribe([](int v) { std::cout << v << " "; });
    // Output: 1 2
    //! [merge]
    return 0;
}
