#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example window_toggle.cpp
 **/
int main()
{

    //! [window_toggle]
    size_t counter{};
    auto source = rpp::source::just(rpp::schedulers::current_thread{}, 1, 2, 3, 4, 5) | rpp::operators::publish() | rpp::operators::ref_count();
    source
        | rpp::operators::window_toggle(source, [source](int){ return source | rpp::ops::filter([](int v) { return v % 2 == 0; }); })
        | rpp::operators::subscribe([&counter](const rpp::window_toggle_observable<int>& obs)
        { 
            std::cout << "New observable " << ++counter << std::endl; 
            obs.subscribe([counter](int v) {std::cout << counter << ": " << v << " " << std::endl; }, [counter]() { std::cout << "closing " << counter << std::endl; }); 
        });
    // Output: 
    // New observable 1
    // 1: 1 
    // New observable 2
    // 1: 2 
    // 2: 2 
    // closing 1
    // New observable 3
    // 2: 3 
    // 3: 3 
    // New observable 4
    // 2: 4 
    // 3: 4 
    // 4: 4 
    // closing 2
    // closing 3
    // New observable 5
    // 4: 5 
    // 5: 5 
    // closing 4
    // closing 5
    //! [window_toggle]

    return 0;
}
