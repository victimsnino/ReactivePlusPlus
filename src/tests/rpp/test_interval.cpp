//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <snitch/snitch.hpp>

#include <rpp/operators/as_blocking.hpp>
#include <rpp/operators/subscribe.hpp>
#include <rpp/operators/take.hpp>
#include <rpp/sources/interval.hpp>
#include <rpp/schedulers/current_thread.hpp>
#include <rpp/schedulers/immediate.hpp>
#include <rpp/schedulers/new_thread.hpp>

#include "mock_observer.hpp"
#include "test_scheduler.hpp"
#include "snitch_logging.hpp"

#include <chrono>

TEST_CASE("interval emit values with provided interval")
{
    auto scheduler = test_scheduler{};
    auto mock      = mock_observer_strategy<size_t>{};
    SECTION("interval observable")
    {
        auto interval     = std::chrono::seconds{1};
        auto obs          = rpp::source::interval(interval, scheduler);
        auto initial_time = test_scheduler::worker_strategy::now();

        SECTION("subscribe on it via take 3")
        {
            obs | rpp::ops::take(3) | rpp::ops::subscribe(mock);
            SECTION("nothing happens immediately till scheduler advanced")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(scheduler.get_schedulings() == std::vector{ initial_time + interval});
                CHECK(scheduler.get_executions().empty());
            }
            SECTION("move time in advance on interval once")
            {
                scheduler.time_advance(interval);
                SECTION("observer obtains first value")
                {
                    CHECK(mock.get_received_values() == std::vector<size_t>{0});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
                SECTION("interval schedules schedulable with provided interval")
                {
                    CHECK(scheduler.get_schedulings() == std::vector{ initial_time + interval,
                          initial_time + 2*interval});
                    CHECK(scheduler.get_executions() == std::vector{initial_time+interval});
                }
            }
            SECTION("move time in advance on interval enough amount of time")
            {
                for (size_t i = 0; i < 5; ++i)
                    scheduler.time_advance(interval);

                SECTION("observer obtains sequence of values")
                {
                    CHECK(mock.get_received_values() == std::vector<size_t>{0, 1, 2});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
                SECTION("interval schedules schedulable with provided interval")
                {
                    CHECK(scheduler.get_executions() == std::vector{ initial_time + interval,
                          initial_time + 2*interval,
                          initial_time + 3*interval});
                    CHECK(scheduler.get_schedulings() == std::vector{ initial_time + interval,
                          initial_time + 2*interval,
                          initial_time + 3*interval});
                }
            }
        }
    }

    SECTION("interval observable with initial delay")
    {
        auto initial_delay = std::chrono::seconds{2};
        auto interval      = std::chrono::seconds{1};
        auto obs           = rpp::source::interval(initial_delay, interval, scheduler);
        auto initial_time  = test_scheduler::worker_strategy::now();

        SECTION("subscribe on it via take 3")
        {
            obs | rpp::ops::take(3) | rpp::ops::subscribe(mock);
            SECTION("nothing happens immediately till scheduler advanced")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(scheduler.get_schedulings() == std::vector{ initial_time + initial_delay});
                CHECK(scheduler.get_executions().empty());
            }

            SECTION("move time in advance on interval enough amount of time")
            {
                for (size_t i = 0; i < 5; ++i)
                    scheduler.time_advance(interval);
                scheduler.time_advance(interval);
                SECTION("observer obtains sequence of values")
                {
                    CHECK(mock.get_received_values() == std::vector<size_t>{0, 1, 2});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
                SECTION("interval schedules schedulable with provided interval")
                {
                    CHECK(scheduler.get_schedulings() == std::vector{ initial_time + initial_delay,
                          initial_time + initial_delay + interval,
                          initial_time + initial_delay + 2 * interval });
                    CHECK(scheduler.get_executions() == std::vector{ initial_time + initial_delay,
                          initial_time + initial_delay + interval,
                          initial_time + initial_delay + 2 * interval });
                }
            }
        }
    }
}
