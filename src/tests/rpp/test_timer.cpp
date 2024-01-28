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

#include <rpp/sources/timer.hpp>

#include "mock_observer.hpp"
#include "test_scheduler.hpp"

#include <chrono>

TEST_CASE("timer emit single value at provided duration")
{
    auto scheduler = test_scheduler{};
    auto mock      = mock_observer_strategy<size_t>{};

    SECTION("timer observable")
    {
        auto when         = std::chrono::seconds{1};
        auto obs          = rpp::source::timer(when, scheduler);
        auto initial_time = test_scheduler::worker_strategy::now();

        SECTION("subscribe")
        {
            obs | rpp::ops::subscribe(mock);

            SECTION("nothing happens immediately till scheduler advanced")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(scheduler.get_schedulings() == std::vector{initial_time + when});
                CHECK(scheduler.get_executions().empty());
            }

            SECTION("advance time")
            {
                scheduler.time_advance(when);

                SECTION("observer obtains value")
                {
                    CHECK(mock.get_received_values() == std::vector<size_t>{0});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }

                SECTION("timer schedules schedulable with provided interval")
                {
                    CHECK(scheduler.get_schedulings() == std::vector{initial_time + when});
                    CHECK(scheduler.get_executions() == std::vector{initial_time + when});
                }
            }
        }
    }
}