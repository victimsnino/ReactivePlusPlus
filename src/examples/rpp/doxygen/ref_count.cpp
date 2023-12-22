#include <rpp/rpp.hpp>
#include <iostream>

/**
 * @example ref_count.cpp
 **/
int main() // NOLINT(bugprone-exception-escape)
{
    {
        //! [ref_count]
        auto observable = rpp::source::create<int>([](const auto& observer)
        {
            std::cout << "SUBSCRIBE" << std::endl;
            for(int i =0; i < 3; ++i) {
                observer.on_next(i);
            }
            observer.on_completed();
        }) | rpp::operators::multicast();

        std::cout << "subscribe first" << std::endl;
        observable.subscribe([](int v) {std::cout << "#1 " << v << std::endl; });
        // No Output

        std::cout << "subscribe with ref_count" << std::endl;
        observable.ref_count().subscribe([](int v) {std::cout << "#2 " << v << std::endl; });
        // Output:
        // subscribe first
        // subscribe with ref_count
        // SUBSCRIBE
        // #1 0
        // #2 0
        // #1 1
        // #2 1
        // #1 2
        // #2 2

        //! [ref_count]
    }
    {
        //! [ref_count_operator]
        auto observable = rpp::source::just(1, 2, 3) | rpp::operators::multicast();
        observable.subscribe([](int v) {std::cout << "#1 " << v << std::endl; });
        // No Output

        observable | rpp::ops::ref_count() | rpp::ops::subscribe([](int v) {std::cout << "#2 " << v << std::endl; });
        // Output:
        // #1 1
        // #2 1
        // #1 2
        // #2 2
        // #1 3
        // #2 3

        //! [ref_count_operator]
    }
    return 0;
}
