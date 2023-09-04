#include <rpp/rpp.hpp>

#include <iostream>

std::ostream& operator<<(std::ostream& out, const std::tuple<int, int>& value)
{
    out << "{" << std::get<0>(value) << "," << std::get<1>(value) << "}";
    return out;
}

/**
 * @example combine_latest.cpp
 **/
int main()
{
    //! [combine_latest]
    rpp::source::just(rpp::schedulers::current_thread{}, 1, 2, 3)                                 // source 1
        | rpp::ops::combine_latest(rpp::source::just(rpp::schedulers::current_thread{}, 4, 5, 6)) // source 2
        | rpp::ops::subscribe([](const std::tuple<int, int>& v) { std::cout << "-" << v; },
                              [](const std::exception_ptr&) {},
                              []() { std::cout << "-|" << std::endl; });
    // source 1:   -1---2---3-|
    // source 2: -4---5---6-| (note that source 2 is subscribed earlier than source 1)
    // Output  : -{1,4}-{1,5}-{2,5}-{2,6}-{3,6}}-|
    //! [combine_latest]

    //! [combine_latest custom combiner]
    rpp::source::just(1, 2, 3)                                                       // source 1
        | rpp::ops::combine_latest([](int left, int right) { return left + right; }, // custom combiner
                                   rpp::source::just(4, 5, 6))                       // source 2
        | rpp::ops::subscribe([](int v) { std::cout << "-" << v; },
                              [](const std::exception_ptr&) {},
                              []() { std::cout << "-|" << std::endl; });
    // source 1:          -1---2---3-|
    // source 2: -4---5---6-| (note that source 2 is subscribed earlier than source 1)
    // Output  : -5-6-7-8-9-|
    //! [combine_latest custom combiner]
    return 0;
}
