#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example throttle.cpp
 **/
int main()
{
    //! [throttle]
    auto start = rpp::schedulers::clock_type::now();
    rpp::source::just(rpp::schedulers::current_thread{}, 1, 2, 5, 6, 9, 10)
            | rpp::operators::flat_map([](int v)
                {
                    return rpp::source::just(v) | rpp::operators::delay(std::chrono::milliseconds(500) * v, rpp::schedulers::current_thread{});
                })
            | rpp::operators::filter([&](int v)
                {
                    std::cout << "> Sent value " << v << " at " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << std::endl;
                    return true;
                })
            | rpp::operators::throttle(std::chrono::milliseconds{700})
            | rpp::operators::subscribe([&](int v) { std::cout << ">>> new value " << v << " at " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << std::endl;},
                                        [](const std::exception_ptr&){},
                                        [&]() { std::cout << ">>> completed at " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << std::endl; });



    // Output:
    // > Sent value 1 at 500
    // >>> new value 1 at 500
    // > Sent value 2 at 1000
    // > Sent value 5 at 2500
    // >>> new value 5 at 2500
    // > Sent value 6 at 3000
    // > Sent value 9 at 4500
    // >>> new value 9 at 4500
    // > Sent value 10 at 5000
    // >>> completed at 5000
    //! [throttle]
    return 0;
}
