#include <rpp/rpp.hpp>

#include <chrono>
#include <ctime>
#include <iostream>

/**
 * \example delay.cpp
 **/
int main()
{
    //! [delay]

    rpp::source::just(1, 2, 3)
            .do_on_next([](auto&& v)
                        {
                            auto emitting_time = rpp::schedulers::clock_type::now();
                            std::cout << "emit " << v << " in thread{" << std::this_thread::get_id() << "} at epoch time " << emitting_time.time_since_epoch().count() << std::endl;
                        })
            .delay(std::chrono::seconds{3}, rpp::schedulers::new_thread{})
            .as_blocking()
            .subscribe([](int v)
                       {
                           auto observing_time = rpp::schedulers::clock_type::now();
                           std::cout << "observe " << v << " in thread{" << std::this_thread::get_id() << "} at epoch time " << observing_time.time_since_epoch().count() << std::endl;
                       });
    // Template for output:
    //    emit 1 in thread{281472967355984} at epoch time 9302615113068
    //    emit 2 in thread{281472967355984} at epoch time 9302615155151
    //    emit 3 in thread{281472967355984} at epoch time 9302615157860
    //    observe 1 in thread{281472962380144} at epoch time 9305618428153
    //    observe 2 in thread{281472962380144} at epoch time 9305618551778
    //    observe 3 in thread{281472962380144} at epoch time 9305618558236
    //! [delay]
    return 0;
}
