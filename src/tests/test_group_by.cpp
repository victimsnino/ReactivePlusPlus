//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/sources/just.hpp>

#include <rpp/operators/group_by.hpp>

SCENARIO("group_by emits grouped seqences of values", "[group_by]")
{
    auto obs = mock_observer<rpp::grouped_observable<int, int, rpp::details::group_by_on_subscribe<int>>>{};
    GIVEN("observable of values")
    {
        auto observable = rpp::source::just(1, 2, 3, 4, 4, 3, 2, 1);
        WHEN("subscribe on it via group_by with identity")
        {
            observable.group_by(std::identity{}).subscribe(obs);
            THEN("Obtained same amount of grouped observables as amount of unique values")
            {
                CHECK(obs.get_total_on_next_count() == 4);
                CHECK(obs.get_on_error_count() == 0);
                CHECK(obs.get_on_completed_count() == 1);
            }
        }
    }
}