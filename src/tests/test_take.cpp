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
#include <rpp/operators/take.hpp>

SCENARIO("take limits count of items")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable")
    {
        auto obs = rpp::source::create<int>([](const auto& sub)
        {
            for (int i = 0; i < 10; ++i)
            {
                auto new_sub = sub; // send it to copy to test for shared
                new_sub.on_next(i);
            }
        });
        WHEN("subscribe on it with take")
        {
            constexpr size_t count   = 5;
            auto             new_obs = obs.take(count);
            new_obs.subscribe(mock);
            THEN("only limited amount of items provided")
            {
                CHECK(mock.get_received_values() == std::vector{ 0, 1, 2, 3, 4 });
                CHECK(mock.get_on_completed_count() == 1);
            }
            auto mock_2 = mock_observer<int>{};
            new_obs.subscribe(mock_2);
            AND_THEN("second subscription see same amount of items")
            {
                CHECK(mock_2.get_received_values() == std::vector{ 0, 1, 2, 3, 4 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    GIVEN("observable with check for subscription")
    {
        size_t loop_count = 0;
        auto obs          = rpp::source::create<int>([&](const auto& sub)
        {
            while (sub.is_subscribed())
            {
                sub.on_next(1);
                ++loop_count;
            }
        });
        WHEN("subscribe on it with take")
        {
            constexpr size_t limit_count = 2;
            obs.take(limit_count).subscribe(mock);
            THEN("it immediately unsubscribed when condition meets")
            {
                CHECK(mock.get_total_on_next_count() == limit_count);
                CHECK(loop_count == limit_count);
            }
        }
    }
}
