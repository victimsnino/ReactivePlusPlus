#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example do.cpp
 **/

int main()
{
    //! [tap_observer]
    rpp::source::just(1, 2)
            .tap(rpp::make_specific_observer<int>([](int v) { std::cout << "(TAP) NEW item " << v << std::endl; },
                                                  []() { std::cout << "(TAP) Completed" << std::endl; }))
            .subscribe([](int v) { std::cout << "NEW item " << v << std::endl; },
                       []() { std::cout << "Completed" << std::endl; });
    // Output:
    // (TAP) NEW item 1
    // NEW item 1
    // (TAP) NEW item 2
    // NEW item 2
    // (TAP) Completed
    // Completed
    //! [tap_observer]

    //! [tap_callbacks]
    rpp::source::just(1, 2)
            .tap([](int               v) { std::cout << "(TAP) NEW item " << v << std::endl; },
                 [](std::exception_ptr) {},
                 []() { std::cout << "(TAP) Completed" << std::endl; })
            .subscribe([](int v) { std::cout << "NEW item " << v << std::endl; },
                       []() { std::cout << "Completed" << std::endl; });
    // Output:
    // (TAP) NEW item 1
    // NEW item 1
    // (TAP) NEW item 2
    // NEW item 2
    // (TAP) Completed
    // Completed
    //! [tap_callbacks]

    //! [do_on_next]
    rpp::source::just(1, 2)
            .do_on_next([](int v) { std::cout << "(TAP) NEW item " << v << std::endl; })
            .subscribe([](int  v) { std::cout << "NEW item " << v << std::endl; },
                       []() { std::cout << "Completed" << std::endl; });
    // Output:
    // (TAP) NEW item 1
    // NEW item 1
    // (TAP) NEW item 2
    // NEW item 2
    // Completed
    //! [do_on_next]

    //! [do_on_error]
    rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}))
            .do_on_error([](std::exception_ptr) { std::cout << "(TAP) NEW error" << std::endl; })
            .subscribe([](int                 ) {}, [](std::exception_ptr) { std::cout << "NEW error" << std::endl; });
    // Output:
    // (TAP) NEW error
    // NEW error
    //! [do_on_error]

    //! [do_on_completed]
    rpp::source::empty<int>()
            .do_on_completed([]() { std::cout << "(TAP) Completed" << std::endl; })
            .subscribe([](int) {}, []() { std::cout << "Completed" << std::endl; });
    // Output:
    // (TAP) Completed
    // Completed
    //! [do_on_completed]
    return 0;
}
