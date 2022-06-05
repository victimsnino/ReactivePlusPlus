//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"
#include "rpp/sources.hpp"

#include <rpp/sources/just.hpp>
#include <rpp/operators/switch_on_next.hpp>

#include <catch2/catch_test_macros.hpp>

SCENARIO("switch_on_next switches observable after obtaining new one", "[operators][switch_on_next]")
{
    auto mock = mock_observer<int>();
    GIVEN("just observable of just observables")
    {
        auto observable = rpp::source::just(rpp::source::just(1), rpp::source::just(2), rpp::source::just(3));
        WHEN("subscribe on it via switch_on_next")
        {
            observable.switch_on_next().subscribe(mock);
            THEN("obtains values as from concat")
            {
                CHECK(mock.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}
