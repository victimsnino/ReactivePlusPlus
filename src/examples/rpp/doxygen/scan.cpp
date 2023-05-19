#include <rpp/rpp.hpp>

#include <iostream>

/**
 * @example scan.cpp
 **/

int main()
{
    //! [scan]
    rpp::source::just(1,2,3)
            | rpp::operators::scan(10, std::plus<int>{})
            | rpp::operators::subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 10 11 13 16
    //! [scan]

     //! [scan_vector]
    rpp::source::just(1,2,3)
            | rpp::operators::scan(std::vector<int>{}, [](std::vector<int>&& seed, int new_value)
            {
                seed.push_back(new_value);
                return std::move(seed);
            })
            | rpp::operators::subscribe([](const std::vector<int>& v)
            {
                std::cout << "vector: ";
                for(int val : v)
                    std::cout << val << " ";
                std::cout << std::endl;
            });
    // Output: vector:
    //         vector: 1
    //         vector: 1 2
    //         vector: 1 2 3
    //! [scan_vector]

    //! [scan_no_seed]
    rpp::source::just(1,2,3)
            | rpp::operators::scan(std::plus<int>{})
            | rpp::operators::subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 1 3 6
    //! [scan_no_seed]
    return 0;
}
