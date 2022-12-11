#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example debounce.cpp
 **/
int main()
{
    {
        //! [debounce]
        auto start = rpp::schedulers::clock_type::now();
        rpp::source::just(rpp::schedulers::current_thread{}, 1, 2, 5, 6, 9, 10)
                .flat_map([](int v)
                {
                    return rpp::source::just(v)
                            .delay(std::chrono::milliseconds(500) * v, rpp::schedulers::current_thread{});
                })
                .tap([&](int v)
                {
                    std::cout << "Sent value " << v << " at " << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() << std::endl;
                })
                .debounce(std::chrono::milliseconds{700}, rpp::schedulers::current_thread{})
                .subscribe([](int v) { std::cout << "new value " << v << std::endl; },
                           []() { std::cout << "completed" << std::endl; });


        // Output:
        // Sent value 1 at 504
        // Sent value 2 at 1009
        // new value 2
        // Sent value 5 at 2505
        // Sent value 6 at 3010
        // new value 6
        // Sent value 9 at 4507
        // Sent value 10 at 5007
        // new value 10
        // completed
        //! [debounce]
    }
    return 0;
}
