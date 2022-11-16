#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example switch_map.cpp
 **/
int main()
{
    //! [switch_map]
    rpp::source::just(1, 2, 3)
            .switch_map([](int val) {
                if (val == 1)
                    return rpp::source::never<int>()
                        .lift([&](rpp::dynamic_subscriber<int> sub) {
                            sub.get_subscription().add([&]() {
                                std::cout << "x-"; // x is notation for unsubscribed
                            });
                            return sub;
                        })
                        .as_dynamic();

                return rpp::source::from_iterable(std::vector{val, val})
                    .as_dynamic();
            })
            .subscribe([](int v) { std::cout << v << "-"; });
    // Output: x-2-2-3-3-
    //! [switch_map]
    return 0;
}
