//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <catch2/catch_test_macros.hpp>

#include <rpp/operators/flat_map.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/never.hpp>

#include "mock_observer.hpp"

SCENARIO("flat_map transforms items and then merge emissions from underlying observables", "[flat_map]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of items")
    {
        auto obs = rpp::source::just(1,2,3);
        WHEN("subscribe on it via flat_map")
        {
            obs.flat_map([](int v){return rpp::source::just(v, v*10);}).subscribe(mock);
            THEN("subscriber obtains values from observables obtained via flat_map")
            {
                CHECK(mock.get_received_values() == std::vector{1,10,2,20,3,30});
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        WHEN("subscribe on it via flat_map with error")
        {
            obs.flat_map([](int){return rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));}).subscribe(mock);
            THEN("subscriber obtains values from observables obtained via flat_map")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
            }
        }
        WHEN("subscribe on it via flat_map with empty")
        {
            obs.flat_map([](int){return rpp::source::empty<int>();}).subscribe(mock);
            THEN("subscriber obtains values from observables obtained via flat_map")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        WHEN("subscribe on it via flat_map with never")
        {
            auto sub = obs.flat_map([](int){return rpp::source::never<int>();}).subscribe(mock);
            THEN("subscriber obtains values from observables obtained via flat_map")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(sub.is_subscribed());
            }
        }
    }
}