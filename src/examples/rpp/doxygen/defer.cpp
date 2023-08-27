#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example defer.cpp
 **/

int main() // NOLINT
{
    //! [defer from_iterable]
    rpp::source::defer([] {
        std::cout << "Observable factory called\n";
        return rpp::source::from_iterable(std::vector<int>{ 1,2,3 }); })
        .subscribe([](int v) { std::cout << v << "\n"; }, rpp::utils::rethrow_error_t{}, []() { std::cout << "On complete\n"; });
    // Output: Observable factory called 
    //         1 
    //         2 
    //         3
    //         On complete
    //! [defer from_iterable]
    
    //! [defer mutable source]
    auto obs = rpp::source::defer([] {
        std::cout << "Observable factory called\n";
        auto inner_obs = rpp::source::create<int>([state = std::make_shared<int>(0)](const auto& obs) {
            obs.on_next((*state)++);
            obs.on_completed();
        });  
        return inner_obs; 
    });
    obs.subscribe([](int v) { std::cout << v << "\n"; }, rpp::utils::rethrow_error_t{}, []() { std::cout << "On complete\n"; });
    obs.subscribe([](int v) { std::cout << v << "\n"; }, rpp::utils::rethrow_error_t{}, []() { std::cout << "On complete\n"; });
    // Output: Observable factory called 
    //         0
    //         On complete
    //         Observable factory called 
    //         0
    //         On complete
    //! [defer mutable source]
}