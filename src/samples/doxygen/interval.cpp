#include <rpp/rpp.hpp>

#include <iostream>
#include <chrono>

/**
 * \example interval.cpp
 **/

int main()
{
    {
        //! [interval]
        auto cur_time = std::chrono::high_resolution_clock::now();

        rpp::source::interval(std::chrono::seconds{ 2 })
            .take(5)
            .subscribe([cur_time](size_t v)
                {
                    auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - cur_time).count();
                    std::cout << "Seconds since start " << diff << " value " << v << std::endl;
                });

        // Output: Seconds since start 2 value 0
        //         Seconds since start 4 value 1
        //         Seconds since start 6 value 2
        //         Seconds since start 8 value 3
        //         Seconds since start 10 value 4
        //! [interval]
    }

    {
        //! [interval_with_first]
        auto cur_time = std::chrono::high_resolution_clock::now();

        rpp::source::interval(std::chrono::seconds{ 1}, std::chrono::seconds{ 2 })
            .take(5)
            .subscribe([cur_time](size_t v)
                {
                    auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - cur_time).count();
                    std::cout << "Seconds since start " << diff << " value " << v << std::endl;
                });

        // Output: Seconds since start 1 value 0
        //         Seconds since start 3 value 1
        //         Seconds since start 5 value 2
        //         Seconds since start 7 value 3
        //         Seconds since start 9 value 4
        //! [interval_with_first]
    }

    return 0;
}
