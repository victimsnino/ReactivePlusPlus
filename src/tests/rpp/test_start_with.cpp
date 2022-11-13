//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <catch2/catch_test_macros.hpp>

#include <rpp/operators/start_with.hpp>

#include <rpp/sources/just.hpp>

#include "mock_observer.hpp"

SCENARIO("start_with works as concat with prepending instead of adding at the end", "[start_with]")
{
    auto mock = mock_observer<int>{};

    auto check = [&]
    {
        THEN("obtain values from start_with firstly, then from original observable")
        {
            CHECK(mock.get_received_values() == std::vector{ 2,3,1 });
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
    };
    GIVEN("3 observables")
    {
        auto obs_1 = rpp::source::just(1);
        auto obs_2 = rpp::source::just(2);
        auto obs_3 = rpp::source::just(3);
        WHEN("subscribe on them via start_with")
        {
            obs_1.start_with(obs_2, obs_3).subscribe(mock);

            check();
        }
    }

    GIVEN("observable")
    {
        auto obs_1 = rpp::source::just(1);
        WHEN("subscribe on it via start_with with values")
        {
            obs_1.start_with(2, 3).subscribe(mock);

            check();
        }
    }
}