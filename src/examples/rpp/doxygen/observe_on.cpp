#include <rpp/rpp.hpp>

#include <chrono>
#include <ctime>
#include <iostream>

/**
 * \example observe_on.cpp
 **/
int main() // NOLINT(bugprone-exception-escape)
{
    //! [observe_on]

    auto start = rpp::schedulers::clock_type::now();

    rpp::source::create<int>([&start](const auto& obs) {
        for (int i = 0; i < 3; ++i)
        {
            auto emitting_time = rpp::schedulers::clock_type::now();
            std::cout << "emit " << i << " in thread{" << std::this_thread::get_id() << "} duration since start " << std::chrono::duration_cast<std::chrono::seconds>(emitting_time - start).count() << "s" << std::endl;

            obs.on_next(i);
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
        auto emitting_time = rpp::schedulers::clock_type::now();
        std::cout << "emit error in thread{" << std::this_thread::get_id() << "} duration since start " << std::chrono::duration_cast<std::chrono::seconds>(emitting_time - start).count() << "s" << std::endl;

        obs.on_error({});
    })
        | rpp::operators::observe_on(rpp::schedulers::new_thread{}, std::chrono::seconds{3})
        | rpp::operators::as_blocking()
        | rpp::operators::subscribe([&](int v) {
                    auto observing_time = rpp::schedulers::clock_type::now();
                    std::cout << "observe " << v << " in thread{" << std::this_thread::get_id() << "} duration since start " << std::chrono::duration_cast<std::chrono::seconds>(observing_time - start).count() <<"s" << std::endl; },
                                    [&](const std::exception_ptr&) {
                                        auto observing_time = rpp::schedulers::clock_type::now();
                                        std::cout << "observe error in thread{" << std::this_thread::get_id() << "} duration since start " << std::chrono::duration_cast<std::chrono::seconds>(observing_time - start).count() << "s" << std::endl;
                                    });

    // Template for output:
    // emit 0 in thread{139800298538880} duration since start 0s
    // emit 1 in thread{139800298538880} duration since start 1s
    // emit 2 in thread{139800298538880} duration since start 2s
    // observe 0 in thread{139800298534464} duration since start 3s
    // emit error in thread{139800298538880} duration since start 3s
    // observe error in thread{139800298538880} duration since start 3s
    //! [observe_on]
    return 0;
}
