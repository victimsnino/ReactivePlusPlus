//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "mock_observer.hpp"
#include "rpp/sources.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/operators/skip.hpp>

SCENARIO("skip ignores first `count` of items", "[skip]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable with 10 items")
    {
        auto obs = rpp::source::create<int>([](const auto& sub)
        {
            for (int i = 0; i < 10; ++i)
            {
                const auto& new_sub = sub; // send it to copy to test for shared
                new_sub.on_next(i);
            }
            sub.on_completed();
        });
        WHEN("subscribe on it with skip(5)")
        {
            constexpr size_t count = 5;
            auto             new_obs = obs.skip(count);
            new_obs.subscribe(mock);
            THEN("skipped first `count` items")
            {
                CHECK(mock.get_received_values() == std::vector{ 5,6,7,8,9});
                CHECK(mock.get_on_completed_count() == 1);
            }
            auto mock_2 = mock_observer<int>{};
            new_obs.subscribe(mock_2);
            AND_THEN("second subscription see same amount of items")
            {
                CHECK(mock_2.get_received_values() == std::vector{ 5,6,7,8,9 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it with skip(0)")
        {
            constexpr size_t count = 0;
            auto             new_obs = obs.skip(count);
            new_obs.subscribe(mock);
            THEN("see all items")
            {
                CHECK(mock.get_received_values() == std::vector{ 0,1,2,3,4,5,6,7,8,9 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it with skip(1000)")
        {
            constexpr size_t count = 1000;
            auto             new_obs = obs.skip(count);
            new_obs.subscribe(mock);
            THEN("no any items except of on_completed")
            {
                CHECK(mock.get_received_values() == std::vector<int>{ });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}
