#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example from.cpp
 **/

int main()
{
    {
        //! [from_iterable]
        std::vector<int> vals{ 1,2,3 };
        rpp::source::from_iterable(vals).subscribe([](int v) {std::cout << v << " "; });
        //! [from_iterable]
    }

    {
        //! [from_iterable with model]
        std::vector<int> vals{ 1,2,3 };
        rpp::source::from_iterable<rpp::memory_model::use_shared>(vals).subscribe([](int v) {std::cout << v << " "; });
        //! [from_iterable with model]
    }

    {
        //! [from_iterable with scheduler]
        std::vector<int> vals{ 1,2,3 };
        rpp::source::from_iterable(vals, rpp::schedulers::immediate{}).subscribe([](int v) {std::cout << v << " "; });
        rpp::source::from_iterable(vals, rpp::schedulers::current_thread{}).subscribe([](int v) {std::cout << v << " "; });
        //! [from_iterable with scheduler]
    }

    // {
    //     //! [from_callable]
    //     rpp::source::from_callable([]() {return 49; }).subscribe([](int v) {std::cout << v << " "; });
    //     // Output: 49
    //     //! [from_callable]
    // }
    return 0;
}