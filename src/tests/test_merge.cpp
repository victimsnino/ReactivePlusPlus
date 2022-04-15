//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "mock_observer.h"

#include <catch2/catch_test_macros.hpp>
#include <rpp/operators/merge.h>
#include <rpp/sources.h>
#include <rpp/observables/dynamic_observable.h>

SCENARIO("merge for observable of observables")
{
    auto mock = mock_observer<int>();
    GIVEN("observable of observables")
    {
        auto obs= rpp::source::just(rpp::source::just(1), rpp::source::just(2));

        WHEN("subscribe on concat of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values FROM underlying observables")
            {
                CHECK(mock.get_received_values() == std::vector{ 1,2 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("observable of observables with first never")
    {
        auto obs = rpp::source::just(rpp::source::never<int>().as_dynamic(), rpp::source::just(2).as_dynamic());

        WHEN("subscribe on concat of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_received_values() == std::vector{ 2 });
                CHECK(mock.get_on_completed_count() == 0); //no complete due to first observable sends nothing
            }
        }
    }
    GIVEN("observable of observables without complete")
    {
        auto obs = rpp::source::create<rpp::dynamic_observable<int>>([](const auto& sub)
        {
            sub.on_next(rpp::source::just(1).as_dynamic());
            sub.on_next(rpp::source::just(2).as_dynamic());
        });

        WHEN("subscribe on concat of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_received_values() == std::vector{ 1, 2 });
                CHECK(mock.get_on_completed_count() == 0); //no complete due to root observable is not completed
            }
        }
    }
    GIVEN("observable of observables one with error")
    {
        auto obs = rpp::source::create<rpp::dynamic_observable<int>>([](const auto& sub)
            {
                sub.on_next(rpp::source::just(1).as_dynamic());
                sub.on_next(rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})).as_dynamic());
                sub.on_next(rpp::source::just(2).as_dynamic());
            });

        WHEN("subscribe on concat of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_received_values() == std::vector{ 1 });
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0); //no complete due to error
            }
        }
    }
    GIVEN("observable of observables with error")
    {
        auto obs = rpp::source::create<rpp::dynamic_observable<int>>([](const auto& sub)
        {
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
            sub.on_next(rpp::source::just(1).as_dynamic());
        });

        WHEN("subscribe on concat of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0); //no complete due to error
            }
        }
    }
}