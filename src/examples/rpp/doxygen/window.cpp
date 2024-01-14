#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example window.cpp
 **/
int main()
{

    //! [window]
    rpp::source::just(1, 2, 3, 4, 5)
        | rpp::operators::window(3)
        | rpp::operators::subscribe([](const rpp::window_observable<int>& v) 
        { 
            std::cout << "\nNew observable " << std::endl; 
            v.subscribe([](int v) {std::cout << v << " "; }); 
        });
    // Output: New observable
    //         1 2 3
    //         New observable
    //         4 5
    //! [window]

    return 0;
}
