#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example defer.cpp
 **/

int main() // NOLINT
{
    //! [interval period]
    rpp::source::interval(std::chrono::milliseconds(10), rpp::schedulers::immediate{})
        | rpp::operators::take(3)
        | rpp::operators::subscribe([](size_t v) { std::cout << v << "\n"; }, rpp::utils::rethrow_error_t{}, []() { std::cout << "On complete\n"; });
    // Output: Observable factory called 
    //         1 
    //         2 
    //         3
    //         On complete
    //! [defer from_iterable]
    
    //! [interval initial+period]
    rpp::source::interval(std::chrono::milliseconds(5), std::chrono::milliseconds(10), rpp::schedulers::immediate{})
        | rpp::operators::take(3)
        | rpp::operators::subscribe([](size_t v) { std::cout << v << "\n"; }, rpp::utils::rethrow_error_t{}, []() { std::cout << "On complete\n"; });
    // Output: Observable factory called 
    //         1 
    //         2 
    //         3
    //         On complete
    //! [defer from_iterable]
}