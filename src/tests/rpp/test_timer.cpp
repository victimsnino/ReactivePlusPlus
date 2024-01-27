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
    auto scheduler  = test_scheduler{};
    auto scheduler2 = test_scheduler{};
    auto mock       = mock_observer_strategy<size_t>{};
    auto mock2      = mock_observer_strategy<size_t>{};

    SECTION("timer observable")
    {
        auto when       = std::chrono::seconds{1};
        auto time_point = rpp::schedulers::clock_type::now() + when;
        auto obs        = rpp::source::timer(when, scheduler);
        auto obs2       = rpp::source::timer(time_point, scheduler2);

        SECTION("subscribe")
        {
            obs | rpp::ops::subscribe(mock);
            obs2 | rpp::ops::subscribe(mock2);

            SECTION("nothing happens immediately till scheduler advanced")
            {
                auto validate = [&](const auto& mock, const auto& scheduler) {
                    CHECK(mock.get_received_values() == std::vector<size_t>{});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);

                    CHECK(scheduler.get_schedulings().size() == 1);
                    CHECK(scheduler.get_executions().empty());
                };
                validate(mock, scheduler);
                validate(mock2, scheduler2);
            }

            SECTION("advance time")
            {
                scheduler.time_advance(when);
                scheduler2.time_advance(when);

                SECTION("observer obtains value")
                {
                    auto validate = [&](const auto& mock) {
                        CHECK(mock.get_received_values() == std::vector<size_t>{0});
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                    };
                    validate(mock);
                    validate(mock2);
                }

                SECTION("timer schedules schedulable with provided interval")
                {
                    CHECK(scheduler.get_executions().size() == 1);
                    CHECK(scheduler2.get_executions().size() == 1);
                }
            }
        }
    }
}

TEST_CASE("timer with negative relative time_point emits immediatly")
{
    auto scheduler = test_scheduler{};
    auto mock      = mock_observer_strategy<size_t>{};

    SECTION("timer observable")
    {
        auto when       = std::chrono::seconds{1};
        auto time_point = rpp::schedulers::clock_type::now() - when;
        auto obs        = rpp::source::timer(time_point, scheduler);

        SECTION("subscribe")
        {
            obs | rpp::ops::subscribe(mock);

            SECTION("timer emitted value")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{0});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);

                CHECK(scheduler.get_schedulings().size() == 1);
                CHECK(scheduler.get_executions().size() == 1);
            }
        }
    }
}