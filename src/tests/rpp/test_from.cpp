//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "mock_observer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/sources.hpp>

SCENARIO("from iterable", "[source][from]")
{
    auto mock = mock_observer<int>();
    GIVEN("observable from iterable")
    {
        auto vals = std::vector{1, 2, 3, 4, 5, 6};
        auto obs  = rpp::source::from_iterable(vals);
        WHEN("subscribe on it")
        {
            obs.subscribe(mock);
            THEN("observer obtains values in the same order")
            {
                CHECK(mock.get_received_values() == vals);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    // GIVEN("observable from iterable with scheduler")
    // {
    //     auto vals     = std::vector{1, 2, 3, 4, 5, 6};
    //     auto run_loop = rpp::schedulers::run_loop{};
    //     auto obs      = rpp::source::from_iterable(vals, run_loop);
    //     WHEN("subscribe on it and dispatch once")
    //     {
    //         obs.subscribe(mock);
    //         run_loop.dispatch();
    //         THEN("observer obtains first value")
    //         {
    //             CHECK(mock.get_received_values() == std::vector{1});
    //             CHECK(mock.get_on_error_count() == 0);
    //             CHECK(mock.get_on_completed_count() == 0);
    //         }
    //         AND_WHEN("dispatch till no events")
    //         {
    //             while (!run_loop.is_empty())
    //                 run_loop.dispatch();
    //
    //             THEN("observer obtains values in the same order")
    //             {
    //                 CHECK(mock.get_received_values() == vals);
    //                 CHECK(mock.get_on_completed_count() == 1);
    //             }
    //         }
    //     }
    //     WHEN("subscribe on it, unsubscribe on first on next and dispatch till not empty")
    //     {
    //         rpp::composite_subscription sub{};
    //         obs.subscribe(sub,
    //                       [&](const auto& v)
    //                       {
    //                           mock.on_next(v);
    //                           sub.unsubscribe();
    //                       });
    //
    //         size_t dispatch_count{};
    //         while (!run_loop.is_empty())
    //         {
    //             ++dispatch_count;
    //             run_loop.dispatch();
    //         }
    //
    //         THEN("observer obtains first value")
    //         {
    //             CHECK(mock.get_received_values() == std::vector{ 1 });
    //             CHECK(mock.get_on_error_count() == 0);
    //             CHECK(mock.get_on_completed_count() == 0);
    //             CHECK(sub.is_subscribed() == false);
    //             CHECK(dispatch_count == 1);
    //         }
    //     }
    // }
    // GIVEN("observable from iterable with exceiption on begin")
    // {
    //     auto run_loop = rpp::schedulers::run_loop{};
    //     auto obs = rpp::source::from_iterable(my_container_with_error{}, run_loop);
    //     WHEN("subscribe on it and dispatch once")
    //     {
    //         obs.subscribe(mock);
    //         run_loop.dispatch();
    //         THEN("observer obtains error")
    //         {
    //             CHECK(mock.get_total_on_next_count() == 0);
    //             CHECK(mock.get_on_error_count() == 1);
    //             CHECK(mock.get_on_completed_count() == 0);
    //         }
    //     }
    // }
}