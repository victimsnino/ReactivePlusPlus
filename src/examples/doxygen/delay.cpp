#include <rpp/rpp.hpp>

#include <chrono>
#include <ctime>
#include <iostream>

/**
 * \example delay.cpp
 **/
int main()
{
    //! [delay]

    auto start = rpp::schedulers::clock_type::now();

    rpp::source::just(1, 2, 3)
            .do_on_next([&](auto&& v)
                        {
                            auto emitting_time = rpp::schedulers::clock_type::now();
                            std::cout << "emit " << v << " in thread{" << std::this_thread::get_id() << "} duration since start " << std::chrono::duration_cast<std::chrono::seconds>(emitting_time - start).count() << "s"<< std::endl;
                        })
            .delay(std::chrono::seconds{3}, rpp::schedulers::new_thread{})
            .as_blocking()
            .subscribe([&](int v)
                       {
                           auto observing_time = rpp::schedulers::clock_type::now();
                           std::cout << "observe " << v << " in thread{" << std::this_thread::get_id() << "} duration since start " << std::chrono::duration_cast<std::chrono::seconds>(observing_time - start).count() <<"s" << std::endl;
                       });
    // Template for output:
    //    emit 1 in thread{11772} duration since start 0s
    //    emit 2 in thread{11772} duration since start 0s
    //    emit 3 in thread{11772} duration since start 0s
    //    observe 1 in thread{15516} duration since start 3s
    //    observe 2 in thread{15516} duration since start 3s
    //    observe 3 in thread{15516} duration since start 3s
    //! [delay]
    return 0;
}
