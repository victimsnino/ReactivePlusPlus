//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"
#include "test_scheduler.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/operators/subscribe_on.hpp>
#include <rpp/schedulers/new_thread_scheduler.hpp>
#include <rpp/sources/error.hpp>

#include <optional>

TEST_CASE("subscribe_on schedules job in another scheduler")
{
    auto mock = mock_observer<int>{};
    auto scheduler = test_scheduler{};
    GIVEN("observable")
    {
        auto                           obs = rpp::source::create<int>([&](const auto& sub)
        {
            sub.on_next(1);
            sub.on_completed();
        });
        WHEN("subscribe on it with subscribe_on")
        {
            obs.subscribe_on(scheduler).subscribe(mock);
            THEN("expect to obtain value via scheduling")
            {
                REQUIRE(mock.get_total_on_next_count() == 1);
                REQUIRE(mock.get_on_completed_count() == 1);
                REQUIRE(scheduler.get_schedulings() == std::vector{ s_current_time });
            }
        }
    }
    GIVEN("observable with error")
    {
        auto obs = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));
        WHEN("subscribe on it with subscribe_on")
        {
            obs.subscribe_on(scheduler).subscribe(mock);
            THEN("expect to obtain error via scheduling")
            {
                REQUIRE(mock.get_total_on_next_count() == 0);
                REQUIRE(mock.get_on_error_count() == 1);
                REQUIRE(mock.get_on_completed_count() == 0);
                REQUIRE(scheduler.get_schedulings() == std::vector{ s_current_time });
            }
        }
    }
}
