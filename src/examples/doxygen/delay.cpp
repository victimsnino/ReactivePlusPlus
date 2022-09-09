#include <rpp/rpp.hpp>

#include <chrono>
#include <iostream>

/**
 * \example delay.cpp
 **/
int main()
{
    //! [delay]
    std::cout << std::this_thread::get_id() << std::endl;
    rpp::source::just(1, 2, 3)
            .delay(std::chrono::seconds{1}, rpp::schedulers::new_thread{})
            .as_blocking()
            .subscribe([](int v) { std::cout << "[" << std::this_thread::get_id() << "] : " << v << std::endl; });
    // Template for output:
    // TH1
    // [TH2]: 1
    // [TH2]: 2
    // [TH2]: 3
    //! [delay]
    return 0;
}
