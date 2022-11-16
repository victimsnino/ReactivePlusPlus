#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example repeat.cpp
 **/
int main()
{
    //! [repeat]
    rpp::source::just(1, 2, 3)
            .repeat(2)
            .subscribe([](int v) { std::cout << v << " "; },
                       []()
                       {
                           std::cout << "completed" << std::endl;
                       });
    // Output: 1 2 3 1 2 3 completed
    //! [repeat]

    //! [repeat_infinitely]
    rpp::source::just(1, 2, 3)
            .repeat()
            .take(10)
            .subscribe([](int v) { std::cout << v << " "; },
                       []()
                       {
                           std::cout << "completed" << std::endl;
                       });
    // Output: 1 2 3 1 2 3 1 2 3 1 completed
    //! [repeat_infinitely]
    return 0;
}
