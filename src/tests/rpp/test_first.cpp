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

#include <vector>

#include <rpp/operators/first.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/never.hpp>

#include "mock_observer.hpp"

SCENARIO("first only emits once", "[first]")
{
    GIVEN("observable of -1-|")
    {
        auto mock = mock_observer<int>{};
        auto obs = rpp::source::just(1);
        obs.first()
            .subscribe(mock);
        THEN("shall see -1-|")
        {
            CHECK(mock.get_received_values() == std::vector{1});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    GIVEN("observable of -1-2-3-|")
    {
        auto mock = mock_observer<int>{};
        auto obs = rpp::source::just(1, 2, 3);
        obs.first()
            .subscribe(mock);
        THEN("shall see -1-|")
        {
            CHECK(mock.get_received_values() == std::vector{1});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    GIVEN("observable of never")
    {
        auto mock = mock_observer<int>{};
        auto obs = rpp::source::never<int>();
        obs.first()
            .subscribe(mock);
        THEN("shall not see neither completed nor error event")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 0);
        }
    }
}

SCENARIO("first forwards error", "[first]")
{
    GIVEN("observable of x-|")
    {
        auto mock = mock_observer<int>{};
        auto obs = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));
        obs.first()
            .subscribe(mock);
        THEN("shall see error and no-completed event")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 1);
        }
    }
}

SCENARIO("first raises error for empty", "[first]")
{
    GIVEN("observable of ---|")
    {
        auto mock = mock_observer<int>{};
        auto obs = rpp::source::empty<int>();
        obs.first()
            .subscribe(mock);
        THEN("shall see -x")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 1);
        }
    }
}
