#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example scan.cpp
 **/

int main()
{
    //! [scan]
    rpp::source::just(1,2,3)
            .scan(0, std::plus<int>{})
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 1 3 6
    //! [scan]

     //! [scan_vector]
    rpp::source::just(1,2,3)
            .scan(std::vector<int>{}, [](std::vector<int>&& seed, int new_value) -> std::vector<int>&&
            {
                seed.push_back(new_value);
                return std::move(seed);
            })
            .subscribe([](const std::vector<int> v)
            {
                std::cout << "vector: ";
                for(int val : v)
                    std::cout << val << " ";
                std::cout << std::endl;
            });
    // Output: vector: 1
    //         vector: 1 2
    //         vector: 1 2 3
    //! [scan_vector]
    return 0;
}
