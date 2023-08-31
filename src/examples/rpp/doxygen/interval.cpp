#include <rpp/rpp.hpp>

#include <chrono>
#include <iostream>

/**
 * \example defer.cpp
 **/

int main() // NOLINT
{
    auto start = rpp::schedulers::clock_type::now();
    //! [interval period]
    rpp::source::interval(std::chrono::milliseconds(10), rpp::schedulers::immediate{})
        | rpp::operators::take(3)
        | rpp::operators::subscribe(
            [&](size_t v) { std::cout << "emit " << v << " duration since start " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << "ms\n"; }, 
            rpp::utils::rethrow_error_t{}, 
            []() { std::cout << "On complete\n"; });
    // Output: Observable factory called 
    //    emit 1 duration since start 1ms
    //    emit 2 duration since start 15ms
    //    emit 3 duration since start 30ms
    //    On complete
    //! [defer from_iterable]
    
    //! [interval initial+period]
    rpp::source::interval(std::chrono::milliseconds(5), std::chrono::milliseconds(10), rpp::schedulers::immediate{})
        | rpp::operators::take(3)
        | rpp::operators::subscribe(
            [start = rpp::schedulers::clock_type::now()](size_t v) { std::cout << "emit " << v << " duration since start " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << "ms\n"; }, 
            rpp::utils::rethrow_error_t{}, 
            []() { std::cout << "On complete\n"; });
    // Output: Observable factory called 
    //    emit 1 duration since start 15ms
    //    emit 2 duration since start 30ms
    //    emit 3 duration since start 45ms
    //    On complete
    //! [defer from_iterable]
}