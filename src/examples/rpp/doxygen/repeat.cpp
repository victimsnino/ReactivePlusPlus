#include <rpp/rpp.hpp>
#include <exception>
#include <iostream>

/**
 * @example repeat.cpp
 **/
int main()
{
    //! [repeat]
    rpp::source::just(1, 2, 3)
            | rpp::operators::repeat(2)
            | rpp::operators::subscribe([](int v) { std::cout << v << " "; },
                                        [](const std::exception_ptr&){},
                                        []() { std::cout << "completed" << std::endl; });
    // Output: 1 2 3 1 2 3 completed
    //! [repeat]

    //! [repeat_infinitely]
    rpp::source::just(1, 2, 3)
            | rpp::operators::repeat()
            | rpp::operators::take(10)
            | rpp::operators::subscribe([](int v) { std::cout << v << " "; },
                                        [](const std::exception_ptr&){},
                                        []()
                                        {
                                            std::cout << "completed" << std::endl;
                                        });
    // Output: 1 2 3 1 2 3 1 2 3 1 completed
    //! [repeat_infinitely]
    return 0;
}