#include "rpp/sources/fwd.hpp"
#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example concat.cpp
 **/

int main() // NOLINT(bugprone-exception-escape)
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

    //! [concat_as_operator]
    rpp::source::just(rpp::source::just(1).as_dynamic(), 
                      rpp::source::just(2).as_dynamic(), 
                      rpp::source::just(1, 2, 3).as_dynamic())
        | rpp::operators::concat()
        | rpp::operators::subscribe([](int v) { std::cout << v << ", "; }, [](const std::exception_ptr&) {}, []() { std::cout << "completed\n"; });
    // Output: 1, 2, 1, 2, 3, completed
    //! [concat_as_operator]
}