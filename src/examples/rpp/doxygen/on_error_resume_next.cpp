#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example on_error_resume_next.cpp
 **/
int main()
{
    //! [on_error_resume_next]
    rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}))
            .on_error_resume_next([](auto&&)
            {
                return rpp::observable::just(1, 2, 3);
            })
            .subscribe([&](int v)
                       {
                           std::cout << "-" << v;
                       },
                       [&](auto&&)
                       {
                           std::cout << "-x";
                       },
                       [&]()
                       {
                           std::cout << "-|" << std::endl;
                       });
    // source: -x
    // output: -1-2-3-|
    //! [on_error_resume_next]
    return 0;
}
