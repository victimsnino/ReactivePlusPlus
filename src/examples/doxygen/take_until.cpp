#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example take_until.cpp
 **/
int main()
{
    //! [take_until]
    auto subject = rpp::subjects::publish_subject<bool>{};

    rpp::source::create<int>([other_subscriber = subject.get_subscriber()](const auto& subscriber)
                             {
                                 subscriber.on_next(1);
                                 subscriber.on_next(2);

                                 // Should see a terminate event after this
                                 other_subscriber.on_next(true);

                                 subscriber.on_next(3);
                             })
        .take_until(subject.get_observable())
        .subscribe([](int v) { std::cout << "-" << v; },
                   [](const std::exception_ptr&) { std::cout << "-x" << std::endl; },
                   []() { std::cout << "-|" << std::endl; });
    // source 1: -1-2-3-|
    // source 2: ----t-
    // Output  : -1-2-|
    //! [take_until]

    //! [take_until terminate]
    rpp::source::never<int>()
        .take_until(rpp::source::error<bool>(std::make_exception_ptr(std::runtime_error{""})))
        .subscribe([](int v) { std::cout << "-" << v; },
                   [](const std::exception_ptr&) { std::cout << "-x" << std::endl; },
                   []() { std::cout << "-|" << std::endl; });
    // source 1: -
    // source 2: -x
    // Output  : -x
    //! [take_until terminate]
    return 0;
}
