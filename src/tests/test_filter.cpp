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

#include <catch2/catch_test_macros.hpp>
#include <rpp/operators/filter.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>

#include "mock_observer.hpp"

SCENARIO("Filter provides only satisfied items", "[operators][filter]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of finite list of items")
    {
        auto observable = rpp::source::just(1, 2, 3, 4, 5);

        WHEN("subscribe on filtered observable")
        {
            observable.filter([](int v)
            {
                return v % 2 == 0;
            }).subscribe(mock);

            THEN("obtained only satisfied items")
            {
                CHECK(mock.get_received_values() == std::vector<int>{2, 4});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }

        WHEN("subscribe on filtered observable and unsubscribe from inside of filter")
        {
            rpp::composite_subscription sub{};
            observable.filter([&](int   v)
            {
                sub.unsubscribe();
                return v % 2 == 0;
            }).subscribe(sub, mock);

            THEN("obtained nothing")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
        WHEN("subscribe on filtered observable and filter throws exception")
        {
            observable.filter([&](int v)
            {
                if (v != -1)
                    throw std::runtime_error{""};
                return v % 2 == 0;
            }).subscribe(mock);

            THEN("obtained error")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
    GIVEN("observable of error")
    {
        auto observable = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));

        WHEN("subscribe on filtered observable")
        {
            observable.filter([](int v)
            {
                return v % 2 == 0;
            }).subscribe(mock);

            THEN("obtained only error")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}


SCENARIO("filter - no extra copies", "[operators][filter][track_copy]")
{
    GIVEN("observable")
    {
        copy_count_tracker tracker{};
        auto               obs = tracker.get_observable().filter([](const copy_count_tracker&) { return true; });
        WHEN("subscribe")
        {
            obs.subscribe([](copy_count_tracker) {});
            THEN("no extra copies")
            {
                REQUIRE(tracker.get_copy_count() == 1); // only one copy to subscriber
                REQUIRE(tracker.get_move_count() == 0);
            }
        }
    }
}


SCENARIO("filter - no extra copies for move", "[operators][filter][track_copy]")
{
    GIVEN("observable")
    {
        copy_count_tracker tracker{};
        auto obs = tracker.get_observable_for_move().filter([](const copy_count_tracker&) { return true; });
        WHEN("subscribe")
        {
            obs.subscribe([](copy_count_tracker) {});
            THEN("no extra copies")
            {
                REQUIRE(tracker.get_copy_count() == 0);
                REQUIRE(tracker.get_move_count() == 1); // only one move to subscriber
            }
        }
    }
}
