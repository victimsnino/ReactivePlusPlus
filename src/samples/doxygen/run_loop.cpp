#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example run_loop.cpp
 **/
int main()
{
    //! [run_loop]
    auto run_loop = rpp::schedulers::run_loop{};
    auto sub = rpp::source::just(10, 15, 20)
        .observe_on(run_loop)
        .subscribe([](int v) { std::cout <<  v << "\n"; });

    // No output

    while (sub.is_subscribed())
    {
        while (run_loop.is_any_ready_schedulable())
            run_loop.dispatch();
    }
    // Output:
    // 10
    // 15
    // 20
    //! [run_loop]
    return 0;
}
