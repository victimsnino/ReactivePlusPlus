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

#include <rpp/operators/with_latest_from.hpp>
#include <rpp/sources/just.hpp>

TEST_CASE("with_latest_from combines observables")
{
    auto mock = mock_observer<std::tuple<int,int>>{};
    GIVEN("observables of the same type")
    {
        auto obs_1 = rpp::source::just(1);
        auto obs_2 = rpp::source::just(2);
        WHEN("subscribe on it via with_latest_from")
        {
            obs_1.with_latest_from([](auto&&...vals) {return std::make_tuple(std::forward<decltype(vals)>(vals)...); }, obs_2).subscribe(mock);
            THEN("obtain tuple of values")
            {
                CHECK(mock.get_received_values() == std::vector{ std::tuple{1,2} });
                CHECK(mock.get_total_on_next_count() == 1);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}