#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example observe_on.cpp
 **/
int main()
{
    //! [observe_on]
    std::cout << std::this_thread::get_id() << std::endl;
    rpp::source::just(10, 15, 20)
            .observe_on(rpp::schedulers::new_thread{})
            .as_blocking()
            .subscribe([](int v) { std::cout << "[" << std::this_thread::get_id() << "] : " << v << "\n"; });
    // Template for output:
    // TH1
    // [TH2]: 10
    // [TH2]: 15
    // [TH2]: 20
    //! [observe_on]
    return 0;
}
