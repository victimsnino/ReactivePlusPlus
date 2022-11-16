#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example sample.cpp
 **/
int main()
{
    //! [sample_with_time]
    rpp::source::interval(std::chrono::milliseconds{100}, rpp::schedulers::trampoline{})
            .sample_with_time(std::chrono::milliseconds{300}, rpp::schedulers::trampoline{})
            .take(5)
            .subscribe([](int v) { std::cout << v << " "; });
    // Output: 1 4 7 10 13
    //! [sample_with_time]
    return 0;
}
