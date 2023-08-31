#include <rpp/rpp.hpp>

#include <chrono>
#include <ctime>
#include <iostream>

/**
 * \example delay.cpp
 **/
int main() // NOLINT
{
    //! [delay]

    auto start = rpp::schedulers::clock_type::now();

    rpp::source::create<int>([&start](const auto& obs)
    {
        for (int i = 0; i < 3; ++i)
        {
            auto emitting_time = rpp::schedulers::clock_type::now();
            std::cout << "emit " << i << " in thread{" << std::this_thread::get_id() << "} duration since start " << std::chrono::duration_cast<std::chrono::seconds>(emitting_time - start).count() << "s"<< std::endl;
            
            obs.on_next(i);
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
        obs.on_completed();
    })
    | rpp::operators::delay(std::chrono::seconds{3}, rpp::schedulers::new_thread{})
    | rpp::operators::as_blocking()
    | rpp::operators::subscribe([&](int v)
                {
                    auto observing_time = rpp::schedulers::clock_type::now();
                    std::cout << "observe " << v << " in thread{" << std::this_thread::get_id() << "} duration since start " << std::chrono::duration_cast<std::chrono::seconds>(observing_time - start).count() <<"s" << std::endl;
                });

    // Template for output:
    // emit 0 in thread{140260327311232} duration since start 0s
    // emit 1 in thread{140260327311232} duration since start 1s
    // emit 2 in thread{140260327311232} duration since start 2s
    // observe 0 in thread{140260327306816} duration since start 3s
    // observe 1 in thread{140260327306816} duration since start 4s
    // observe 2 in thread{140260327306816} duration since start 5s
    //! [delay]
    return 0;
}
