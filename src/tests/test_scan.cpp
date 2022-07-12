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

#include <rpp/operators/scan.hpp>

#include <rpp/sources/just.hpp>

SCENARIO("scan scans values and store state", "[scan]")
{
    GIVEN("observable")
    {
        auto obs = rpp::observable::just(1,2,3);
        WHEN("subscribe on it via scan with plus")
        {
             auto mock = mock_observer<int>{};

             obs.scan(0, [](auto f, auto s){return f+s;}).subscribe(mock);
            THEN("observer obtains partial sums")
            {
                CHECK(mock.get_received_values() == std::vector{1,3,6});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}