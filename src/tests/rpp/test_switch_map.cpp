//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//                     TC Wang 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <catch2/catch_test_macros.hpp>

#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/switch_map.hpp>
#include <rpp/operators/lift.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/never.hpp>

#include "mock_observer.hpp"

SCENARIO("switch_map acts like flat_map except it unsubscribes the previous source when new source emits", "[switch_map]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of items")
    {
        auto obs = rpp::source::just(rpp::schedulers::immediate{}, 1,2,3);
        WHEN("subscribe on it via switch_map")
        {
            obs.switch_map([](int v){return rpp::source::just(rpp::schedulers::immediate{}, v, v*10);})
                .subscribe(mock);
            THEN("subscriber obtains values from observables obtained via switch_map")
            {
                CHECK(mock.get_received_values() == std::vector{1,10,2,20,3,30});
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        WHEN("subscribe on it via switch_map with error")
        {
            obs.switch_map([](int){return rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));})
                .subscribe(mock);
            THEN("subscriber obtains values from observables obtained via switch_map")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
            }
        }
        WHEN("subscribe on it via switch_map with empty")
        {
            obs.switch_map([](int){return rpp::source::empty<int>();})
                .subscribe(mock);
            THEN("subscriber obtains values from observables obtained via switch_map")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        WHEN("subscribe on it via switch_map with never")
        {
            auto sub = obs.switch_map([](int){return rpp::source::never<int>();})
                .subscribe(mock);
            THEN("subscriber obtains values from observables obtained via switch_map")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
            }
            THEN("subscription is still subscribed since no inner observable completes")
            {
                CHECK(sub.is_subscribed());
            }
        }
        WHEN("subscribe on it via switch_map with [never, just...]")
        {
            size_t count_of_unsubscribed{0};

            obs.switch_map([&](int val) {
                    if (val == 1) {
                        return rpp::source::never<int>()
                            // "lift" is used for capturing the unsubscribe event
                            .lift([&](rpp::dynamic_subscriber<int> sub) {
                                sub.get_subscription().add([&]() {
                                    ++count_of_unsubscribed;
                                });
                                return sub;
                            })
                            .as_dynamic();
                    }

                    return rpp::source::just(rpp::schedulers::immediate{}, 2 * val).as_dynamic();
                })
                .subscribe(mock);
            THEN("subscriber observes last two emission")
            {
                CHECK(mock.get_received_values() == std::vector{4, 6});
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
            THEN("the first never-observable shall be unsubscribed")
            {
                CHECK(count_of_unsubscribed == 1);
            }
        }
        WHEN("subscribe on it via switch_map with [just..., never]")
        {
            auto subscription = obs.switch_map([&](int val) {
                                        if (val == 3) {
                                            return rpp::source::never<int>()
                                                    .as_dynamic();
                                        }

                                        return rpp::source::just(rpp::schedulers::immediate{}, 2 * val).as_dynamic();
                                    })
                                    .subscribe(mock);
            THEN("subscriber observes first two emission and shall not see complete event")
            {
                CHECK(mock.get_received_values() == std::vector{2, 4});
            }
            THEN("subscriber shall not see a complete event because most-recently inner never-observable doesn't complete")
            {
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
            }
            THEN("subscription is still subscribed since most-recently inner never-observable doesn't complete")
            {
                CHECK(subscription.is_subscribed());
            }
        }
    }
}
