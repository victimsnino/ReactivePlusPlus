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

    //! [average]
    rpp::source::just(1,2,3)
            .average()
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 2
    //! [average]

    //! [sum]
    rpp::source::just(1,2,3)
            .sum()
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 6
    //! [sum]

    //! [count]
    rpp::source::just(1,2,3)
            .count()
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 3
    //! [count]

    //! [min]
    rpp::source::just(5,1,2,3)
            .min()
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 1
    //! [min]

    //! [max]
    rpp::source::just(5,1,2,3)
            .max()
            .subscribe([](int v) { std::cout << v << std::endl; });
    // Output: 5
    //! [max]
    return 0;
}
