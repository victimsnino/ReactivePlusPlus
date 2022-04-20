//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "copy_count_tracker.hpp"

#include <catch2/catch_test_macros..hpp>
#include <rpp/observables.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/subscribers.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/observers/state_observer.hpp>

SCENARIO("Observable can be lifted")
{
    auto verifier = copy_count_tracker{};
    auto validate = [&](auto observable)
    {
        WHEN("Call lift")
        {
            int  calls_internal = 0;

            auto new_observable = observable.template lift<int>([&calls_internal](int val, const auto& sub)
            {
                ++calls_internal;
                sub.on_next(val);
            });

            AND_WHEN("subscribe unsubscribed subscriber")
            {
                int calls_external = 0;

                auto subscriber = rpp::specific_subscriber{[&](int) { ++calls_external; }};
                subscriber.unsubscribe();

                new_observable.subscribe(subscriber);

                THEN("No any calls obtained")
                {
                    CHECK(calls_internal == 0);
                    CHECK(calls_external == 0);
                }
            }
        }
        WHEN("Call lift as lvalue")
        {
            auto initial_copy_count = verifier.get_copy_count();
            auto initial_move_count = verifier.get_move_count();

            auto new_observable = observable.lift([](rpp::dynamic_subscriber<double> sub)
            {
                return rpp::dynamic_subscriber{sub.get_subscription(),
                                               [sub](int val)
                                               {
                                                   sub.on_next(static_cast<double>(val) / 2);
                                               }};
            });
            THEN("On subscribe obtain modified values")
            {
                std::vector<double>                 obtained_values{};
                new_observable.subscribe([&](double v) { obtained_values.push_back(v); });

                CHECK(obtained_values == std::vector{5.0, 2.5});
            }
            if constexpr (!std::is_same_v<decltype(observable), rpp::dynamic_observable<int>>)
            {
                AND_THEN("One copy to lambda + one move to new observable")
                {
                    CHECK(verifier.get_copy_count() == initial_copy_count +1);
                    CHECK(verifier.get_move_count() == initial_move_count +1);
                }
            }
        }
        WHEN("Call lift as rvalue")
        {
            auto initial_copy_count = verifier.get_copy_count();
            auto initial_move_count = verifier.get_move_count();

            auto new_observable = std::move(observable).lift([](rpp::dynamic_subscriber<double> sub)
            {
                return rpp::specific_subscriber{sub.get_subscription(),
                                                [sub](int val)
                                                {
                                                    sub.on_next(static_cast<double>(val) / 2);
                                                }};
            });
            THEN("On subscribe obtain modified values")
            {
                std::vector<double>                 obtained_values{};
                new_observable.subscribe([&](double v) { obtained_values.push_back(v); });

                CHECK(obtained_values == std::vector{5.0, 2.5});
            }
            if constexpr (!std::is_same_v<decltype(observable), rpp::dynamic_observable<int>>)
            {
                AND_THEN("One move to lambda + one move to new observable")
                {
                    CHECK(verifier.get_copy_count() == initial_copy_count);
                    CHECK(verifier.get_move_count() == initial_move_count +2);
                }
            }
        }
    };


    auto observable = rpp::observable::create<int>([verifier](const auto& sub)
    {
        sub.on_next(10);
        sub.on_next(5);
        sub.on_completed();
    });

    GIVEN("Observable")
        validate(observable);

    GIVEN("DynamicObservable")
        validate(observable.as_dynamic());
}