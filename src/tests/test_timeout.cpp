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

#include <rpp/sources/interval.hpp>
#include <rpp/sources/never.hpp>
#include <rpp/operators/timeout.hpp>
#include <rpp/operators/concat.hpp>
#include <rpp/operators/take.hpp>

SCENARIO("timeout sends error only on timeout", "[operators][timeout]")
{
    auto mock               = mock_observer<size_t>{};
    auto interval_scheduler = test_scheduler{};
    auto timeout_scheduler  = test_scheduler{};
    auto start_time         = test_scheduler::worker_strategy::now();

    GIVEN("interval observable")
    {
        constexpr rpp::schedulers::duration interval_duration = std::chrono::milliseconds{10};
        auto                                obs               = rpp::source::interval(interval_duration, interval_scheduler);
        WHEN("subscribe on it via timeout with period > interval period")
        {
            constexpr auto timeout_duration = std::chrono::milliseconds{15};
            auto           sub              = obs.take(5).timeout(timeout_duration, timeout_scheduler).subscribe(mock);
            while (sub.is_subscribed())
            {
                interval_scheduler.time_advance(std::chrono::milliseconds{1});
                timeout_scheduler.time_advance(rpp::schedulers::duration{}); // empty due to actually test_scheduler uses same global time, but each of them drains different queues
            }

            THEN("no any timeout events")
            {
                CHECK(mock.get_received_values() == std::vector <size_t>{0, 1, 2, 3, 4});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(interval_scheduler.get_executions() ==
                      std::vector{start_time + interval_duration,
                                  start_time + 2 * interval_duration,
                                  start_time + 3 * interval_duration,
                                  start_time + 4 * interval_duration,
                                  start_time + 5 * interval_duration});
                CHECK(timeout_scheduler.get_executions() ==
                      std::vector{start_time + timeout_duration,
                                  start_time + interval_duration + timeout_duration,
                                  start_time + 2 * interval_duration + timeout_duration,
                                  start_time + 3 * interval_duration + timeout_duration});
            }
        }
        WHEN("subscribe on it via timeout with period < interval period")
        {
            constexpr auto timeout_duration = std::chrono::milliseconds{5};
            auto           sub              = obs.take(5).timeout(timeout_duration, timeout_scheduler).subscribe(mock);
            while (sub.is_subscribed())
            {
                interval_scheduler.time_advance(std::chrono::milliseconds{1});
                timeout_scheduler.time_advance(rpp::schedulers::duration{}); // empty due to actually test_scheduler
                                                                             // uses same global time, but each of them
                                                                             // drains different queues
            }

            THEN("timeout event happens before interval event")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{});
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(interval_scheduler.get_executions() == std::vector<rpp::schedulers::time_point>{});
                CHECK(timeout_scheduler.get_executions() == std::vector{start_time + timeout_duration});
            }
        }
        WHEN("subscribe on it via timeout with period > interval period, but interval ends without completion")
        {
            constexpr auto timeout_duration = std::chrono::milliseconds{15};
            auto sub = obs.take(5).concat_with(rpp::source::never<size_t>()).timeout(timeout_duration, timeout_scheduler).subscribe(mock);
            while (sub.is_subscribed())
            {
                interval_scheduler.time_advance(std::chrono::milliseconds{1});
                timeout_scheduler.time_advance(rpp::schedulers::duration{}); // empty due to actually test_scheduler
                                                                             // uses same global time, but each of them
                                                                             // drains different queues
            }

            THEN("timeout event after last emission + timeout duration")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{0, 1, 2, 3, 4});
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(interval_scheduler.get_executions() ==
                      std::vector{start_time + interval_duration,
                                  start_time + 2 * interval_duration,
                                  start_time + 3 * interval_duration,
                                  start_time + 4 * interval_duration,
                                  start_time + 5 * interval_duration});
                CHECK(timeout_scheduler.get_executions() ==
                      std::vector{start_time + timeout_duration,
                                  start_time + interval_duration + timeout_duration,
                                  start_time + 2 * interval_duration + timeout_duration,
                                  start_time + 3 * interval_duration + timeout_duration,
                                  start_time + 4 * interval_duration + timeout_duration,
                                  start_time + 5 * interval_duration + timeout_duration});
            }
        }
    }
}
