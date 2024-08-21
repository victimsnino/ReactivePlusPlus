#include <rpp/rpp.hpp>

#include <exception>
#include <iostream>

/**
 * @example retry.cpp
 **/
int main()
{
    //! [retry]
    rpp::source::concat(rpp::source::just(1, 2, 3), rpp::source::error<int>({}))
        | rpp::operators::retry(2)
        | rpp::operators::subscribe([](int v) { std::cout << v << " "; },
                                    [](const std::exception_ptr&) { std::cout << "error" << std::endl; },
                                    []() { std::cout << "completed" << std::endl; });
    // Output: 1 2 3 1 2 3 1 2 3 error
    //! [retry]

    //! [retry_infinitely]
    rpp::source::concat(rpp::source::just(1, 2, 3), rpp::source::error<int>({}))
        | rpp::operators::retry()
        | rpp::operators::take(10)
        | rpp::operators::subscribe([](int v) { std::cout << v << " "; },
                                    [](const std::exception_ptr&) { std::cout << "error" << std::endl; },
                                    []() { std::cout << "completed" << std::endl; });
    // Output: 1 2 3 1 2 3 1 2 3 1 completed
    //! [retry_infinitely]
    return 0;
}
