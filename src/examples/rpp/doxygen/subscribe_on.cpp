#include <rpp/rpp.hpp>
#include <iostream>

/**
 * @example subscribe_on.cpp
 **/
int main()
{
    //! [subscribe_on]
    std::cout << std::this_thread::get_id() << std::endl;
    rpp::source::create<int>([](const auto& sub)
            {
                std::cout << "on_subscribe thread " << std::this_thread::get_id() << std::endl;
                sub.on_next(1);
                sub.on_completed();
            })
            | rpp::operators::subscribe_on(rpp::schedulers::new_thread{})
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([](int v) { std::cout << "[" << std::this_thread::get_id() << "] : " << v << "\n"; });
    std::cout << std::this_thread::get_id() << std::endl;

    // Template for output:
    // TH1
    // on_subscribe thread TH2
    // [TH2]: 1
    // TH1
    //! [subscribe_on]
    return 0;
}
