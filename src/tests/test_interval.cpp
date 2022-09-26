//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus


#include <rpp/sources/interval.hpp>
#include <rpp/operators/take.hpp>

#include <catch2/catch_test_macros.hpp>

#include "test_scheduler.hpp"
#include "mock_observer.hpp"


SCENARIO("interval emit values with provided interval", "[interval]")
{
    auto scheduler = test_scheduler{};
    auto mock      = mock_observer<size_t>{};
    GIVEN("interval observable")
    {
        auto interval     = std::chrono::seconds{1};
        auto obs          = rpp::source::interval(interval, scheduler);
        auto initial_time = test_scheduler::worker_strategy::now();

        WHEN("subscribe on it via take 3")
        {
            obs.take(3).subscribe(mock);
            THEN("nothing happens immediately till scheduler advanced")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(scheduler.get_schedulings() == std::vector{ initial_time + interval});
                CHECK(scheduler.get_executions().empty());
            }
            AND_WHEN("move time in advance on interval once")
            {
                scheduler.time_advance(interval);
                THEN("observer obtains first value")
                {
                    CHECK(mock.get_received_values() == std::vector<size_t>{0});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
                THEN("interval schedules schedulable with provided interval")
                {
                    CHECK(scheduler.get_schedulings() == std::vector{ initial_time + interval,
                          initial_time + 2*interval});
                    CHECK(scheduler.get_executions() == std::vector{initial_time+interval});
                }
            }
            AND_WHEN("move time in advance on interval enough amount of time")
            {
                for (size_t i = 0; i < 5; ++i)
                    scheduler.time_advance(interval);

                THEN("observer obtains sequence of values")
                {
                    CHECK(mock.get_received_values() == std::vector<size_t>{0, 1, 2});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
                THEN("interval schedules schedulable with provided interval")
                {
                    CHECK(scheduler.get_executions() == std::vector{ initial_time + interval,
                          initial_time + 2*interval,
                          initial_time + 3*interval});
                    CHECK(scheduler.get_schedulings() == std::vector{ initial_time + interval,
                          initial_time + 2*interval,
                          initial_time + 3*interval,
                          initial_time + 4*interval});
                }
            }
        }
    }

    GIVEN("interval observable with initial delay")
    {
        auto initial_delay = std::chrono::seconds{2};
        auto interval      = std::chrono::seconds{1};
        auto obs           = rpp::source::interval(initial_delay, interval, scheduler);
        auto initial_time  = test_scheduler::worker_strategy::now();

        WHEN("subscribe on it via take 3")
        {
            obs.take(3).subscribe(mock);
            THEN("nothing happens immediately till scheduler advanced")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(scheduler.get_schedulings() == std::vector{ initial_time + initial_delay});
                CHECK(scheduler.get_executions().empty());
            }

            AND_WHEN("move time in advance on interval enough amount of time")
            {
                for (size_t i = 0; i < 5; ++i)
                    scheduler.time_advance(interval);
                scheduler.time_advance(interval);
                THEN("observer obtains sequence of values")
                {
                    CHECK(mock.get_received_values() == std::vector<size_t>{0, 1, 2});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
                THEN("interval schedules schedulable with provided interval")
                {
                    CHECK(scheduler.get_schedulings() == std::vector{ initial_time + initial_delay,
                          initial_time + initial_delay + interval,
                          initial_time + initial_delay + 2 * interval,
                          initial_time + initial_delay + 3 * interval });
                    CHECK(scheduler.get_executions() == std::vector{ initial_time + initial_delay,
                          initial_time + initial_delay + interval,
                          initial_time + initial_delay + 2 * interval });
                }
            }
        }
    }
}
