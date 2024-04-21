#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example thread_pool.cpp
 **/

int main() // NOLINT(bugprone-exception-escape)
{
    //! [thread_pool]
    const auto scheduler = rpp::schedulers::thread_pool{4};
    rpp::source::just(1, 2, 3, 4, 5, 6, 7, 8)
        | rpp::operators::flat_map([scheduler](int value) { return rpp::source::just(scheduler, value)
                                                                 | rpp::operators::delay(std::chrono::nanoseconds{500}, rpp::schedulers::immediate{}); })
        | rpp::operators::as_blocking()
        | rpp::operators::subscribe([](int v) { std::cout << "[" << std::this_thread::get_id() << "] " << v << std::endl; });

    // Output: (can be in any order but same correlation between thread and values)
    // [thread_1] 1
    // [thread_2] 2
    // [thread_3] 3
    // [thread_4] 4
    // [thread_1] 5
    // [thread_2] 6
    // [thread_3] 7
    // [thread_4] 8
    //! [thread_pool]

    //! [computational]
    rpp::source::just(1, 2, 3, 4, 5, 6, 7, 8)
        | rpp::operators::flat_map([](int value) { return rpp::source::just(rpp::schedulers::computational{}, value)
                                                                 | rpp::operators::delay(std::chrono::nanoseconds{500}, rpp::schedulers::immediate{}); })
        | rpp::operators::as_blocking()
        | rpp::operators::subscribe([](int v) { std::cout << "[" << std::this_thread::get_id() << "] " << v << std::endl; });

    // Output: (can be in any order but same correlation between thread and values)
    // [thread_1] 1
    // [thread_2] 2
    // [thread_3] 3
    // [thread_4] 4
    // [thread_1] 5
    // [thread_2] 6
    // [thread_3] 7
    // [thread_4] 8
    //! [computational]

    return 0;
}
