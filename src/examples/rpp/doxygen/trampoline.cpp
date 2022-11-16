#include <rpp/rpp.hpp>
#include <iostream>

std::ostream & operator<<(std::ostream& out, std::tuple<int, int> value)
{
    out << "{" << std::get<0>(value) << "," << std::get<1>(value) << "}";
    return out;
}

/**
 * \example trampoline.cpp
 **/
int main()
{
    //! [trampoline]
    rpp::source::just(rpp::schedulers::trampoline{}, 1, 2, 3)
            .merge_with(rpp::source::just(rpp::schedulers::trampoline{}, 4, 5, 6))
            .subscribe([](const int& v) { std::cout << "-" << v; },
                       []() { std::cout << "-|" << std::endl; });
    // Source 1: -1-2-3-|
    // Source 2:        -4-5-6-|
    // Output  : -1-4-2-5-3-6-|

//    // TODO: Enable this when we have "combine_latest" operator
//    rpp::source::just(rpp::schedulers::trampoline{}, 1, 2, 3)
//        .with_latest_from(rpp::source::just(rpp::schedulers::trampoline{}, 4, 5, 6))
//        .observe_on(rpp::schedulers::trampoline{})
//        .subscribe(
//            [](const auto &v) { std::cout << v << "-"; },
//            [](const std::exception_ptr &error) {},
//            []() { std::cout << "|" << std::endl; });
//    // Source: -1-2-3--|
//    //         -1-2-3--|
//    // Output: {1,1}-{2,2}-{3,3}-|
    //! [trampoline]
    return 0;
}
