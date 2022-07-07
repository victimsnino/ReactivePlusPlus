#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example group_by.cpp
 **/
int main()
{
    //! [group_by]
    rpp::source::just(1, 2, 3, 4, 5, 6, 7, 8).group_by([](int v) { return v % 2 == 0; }).
                                              subscribe([](auto grouped_observable)
                                              {
                                                  grouped_observable.subscribe([key = grouped_observable.get_key()](int val){std::cout << "key [" << key << "] Val: " << val << std::endl;});
                                              });
    // Output: key [0] Val: 1
    //         key [1] Val: 2
    //         key [0] Val: 3
    //         key [1] Val: 4
    //         key [0] Val: 5
    //         key [1] Val: 6
    //         key [0] Val: 7
    //         key [1] Val: 8
    //! [group_by]
    return 0;
}
