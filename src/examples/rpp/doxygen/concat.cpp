#include "rpp/sources/fwd.hpp"
#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example concat.cpp
 **/

int main()
{
    //! [concat_as_source]
    rpp::source::concat(rpp::source::just(1), rpp::source::just(2), rpp::source::just(1,2,3)).subscribe([](int v){std::cout << v << ", ";}, [](const std::exception_ptr&){}, [](){std::cout << "completed\n";});
    // Output: 1, 2, 1, 2, 3, completed
    //! [concat_as_source]

    //! [concat_as_source_vector]
    auto observables = std::vector{rpp::source::just(1), rpp::source::just(2)};
    rpp::source::concat<rpp::memory_model::use_shared>(observables).subscribe([](int v){std::cout << v << ", ";}, [](const std::exception_ptr&){}, [](){std::cout << "completed\n";});
    // Output: 1, 2, completed
    //! [concat_as_source_vector]
}