#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example reduce.cpp
 **/

int main()
{
    //! [reduce]
    rpp::source::just(1,2,3)
            .reduce(0, std::plus<int>{})
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 6
    //! [reduce]

     //! [reduce_vector]
    rpp::source::just(1,2,3)
            .reduce(std::vector<int>{}, [](std::vector<int>&& seed, int new_value)
            {
                seed.push_back(new_value);
                return std::move(seed);
            })
            .subscribe([](const std::vector<int>& v)
            {
                std::cout << "vector: ";
                for(int val : v)
                    std::cout << val << " ";
                std::cout << std::endl;
            });
    // Output: vector: 1 2 3
    //! [reduce_vector]
    return 0;
}
