#include <rpp/rpp.hpp>

#include <chrono>
#include <ctime>
#include <iostream>

/**
 * @example timeout.cpp
 **/

int main() // NOLINT(bugprone-exception-escape)
{
    {
        //! [fallback_observable]
        auto start = rpp::schedulers::clock_type::now();

        rpp::source::just(10, 30, 90, 110)
            | rpp::operators::flat_map([](int v) {
                  return rpp::source::just(v) | rpp::operators::delay(std::chrono::milliseconds{v}, rpp::schedulers::current_thread{});
              })
            | rpp::operators::timeout(std::chrono::milliseconds{35}, rpp::source::just(rpp::schedulers::immediate{}, 0), rpp::schedulers::new_thread{})
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([start](int v) { std::cout << "received " << v << " at " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << std::endl; },
                                        [start](const std::exception_ptr&) {
                                            std::cout << "received error at " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << std::endl;
                                        });
        //! [fallback_observable]
    }

    {
        //! [default]
        auto start = rpp::schedulers::clock_type::now();

        rpp::source::just(10, 30, 90, 110)
            | rpp::operators::flat_map([](int v) {
                  return rpp::source::just(v) | rpp::operators::delay(std::chrono::milliseconds{v}, rpp::schedulers::current_thread{});
              })
            | rpp::operators::timeout(std::chrono::milliseconds{35}, rpp::schedulers::new_thread{})
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([start](int v) { std::cout << "received " << v << " at " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << std::endl; },
                                        [start](const std::exception_ptr&) {
                                            std::cout << "received error at " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << std::endl;
                                        });
        //! [default]
    }
}