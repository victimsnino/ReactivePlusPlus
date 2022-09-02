//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <catch2/catch_test_macros.hpp>

#include <rpp/operators/take_last.hpp>
#include <rpp/operators/merge.hpp>

#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/never.hpp>

#include <rpp/observables/dynamic_observable.hpp>

#include "mock_observer.hpp"

SCENARIO("take_last sends last values in correct order on completed", "[operators][take_last]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of +-1-2-3-4-5-|")
    {
        auto obs = rpp::source::just(1, 2, 3, 4, 5);
        WHEN("subscribe on it via take_last(1)")
        {
            obs.take_last(1).subscribe(mock);
            THEN("see +-5-|")
            {
                CHECK(mock.get_received_values() == std::vector{5});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }

        WHEN("subscribe on it via take_last(3)")
        {
            obs.take_last(3).subscribe(mock);
            THEN("see +-3-4-5-|")
            {
                CHECK(mock.get_received_values() == std::vector{3,4,5});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }

        WHEN("subscribe on it via take_last(5)")
        {
            obs.take_last(5).subscribe(mock);
            THEN("see +-1-2-3-4-5-|")
            {
                CHECK(mock.get_received_values() == std::vector{1,2,3,4,5});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }

        WHEN("subscribe on it via take_last(10)")
        {
            obs.take_last(10).subscribe(mock);
            THEN("see +-1-2-3-4-5-|")
            {
                CHECK(mock.get_received_values() == std::vector{1,2,3,4,5});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via take_last(0)")
        {
            obs.take_last(0).subscribe(mock);
            THEN("see +-|")
            {
                CHECK(mock.get_received_values().empty());
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
};


SCENARIO("take_last forwards error", "[operators][take_last]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of +-x")
    {
        auto source = rpp::source::error<int>(std::exception_ptr{});
        WHEN("subscribe on it via take(2)")
        {
            source.take_last(2).subscribe(mock);
            THEN("see error")
            {
                CHECK(mock.get_received_values().empty());
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }

    GIVEN("observable of +-1-x")
    {
        auto source = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                        rpp::source::error<int>(std::exception_ptr{}).as_dynamic())
                          .merge();

        WHEN("subscribe on it via take(2)")
        {
            source.take_last(2).subscribe(mock);
            THEN("see error")
            {
                CHECK(mock.get_received_values().empty());
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("take_last completes with empty", "[operators][take_last]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of +-|")
    {
        auto source = rpp::source::empty<int>();
        WHEN("subscribe on it via take(2)")
        {
            source.take_last(2).subscribe(mock);
            THEN("see completed")
            {
                CHECK(mock.get_received_values().empty());
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

SCENARIO("take_last nothing with never", "[operators][take_last]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of +-|")
    {
        auto source = rpp::source::never<int>();
        WHEN("subscribe on it via take(2)")
        {
            source.take_last(2).subscribe(mock);
            THEN("see nothing")
            {
                CHECK(mock.get_received_values().empty());
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}
