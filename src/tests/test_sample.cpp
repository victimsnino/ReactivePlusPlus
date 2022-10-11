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
#include <rpp/observables/dynamic_observable.hpp>

#include <rpp/operators/concat.hpp>
#include <rpp/operators/do.hpp>
#include <rpp/operators/take.hpp>
#include <rpp/operators/sample.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/interval.hpp>

SCENARIO("sample throttles emissions", "[operators][sample]")
{
    auto mock = mock_observer<size_t>{};
    auto interval_scheduler = test_scheduler{};
    auto sample_scheduler = test_scheduler{};
    auto start_time = test_scheduler::worker_strategy::now();
    GIVEN("interval observable")
    {
        constexpr rpp::schedulers::duration interval_duration = std::chrono::milliseconds{1};
        auto obs = rpp::source::interval(interval_duration, interval_scheduler);
        WHEN("subscribe on it via sample with period == interval period")
        {
            auto sample_period =interval_duration;
            auto sub = obs.take(3).sample_with_time(sample_period, sample_scheduler).subscribe(mock);
            while(sub.is_subscribed())
            {
                interval_scheduler.time_advance(sample_period);
                sample_scheduler.time_advance(rpp::schedulers::duration{}); // empty due to actually test_schedulers use same global time, but each of them drains different queues 
            }

            THEN("emissions happens as soon as item appears")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{0, 1, 2});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);

                CHECK(interval_scheduler.get_schedulings() ==
                      std::vector{start_time + 1 * interval_duration,
                                  start_time + 2 * interval_duration,
                                  start_time + 3 * interval_duration});
                CHECK(sample_scheduler.get_schedulings() ==
                      std::vector{start_time + 1 * sample_period,
                                  start_time + 2 * sample_period,
                                  start_time + 3 * sample_period});

                CHECK(interval_scheduler.get_executions() ==
                      std::vector{start_time + 1 * interval_duration,
                                  start_time + 2 * interval_duration,
                                  start_time + 3 * interval_duration});
                // last item arrived immediately with on_completed BEFORE actually this scheduler does any action
                CHECK(sample_scheduler.get_executions() == std::vector{start_time + sample_period, start_time + 2 * sample_period}); 
            }
        }
        WHEN("subscribe on it via sample with period <= interval period")
        {
            auto sample_period = rpp::schedulers::duration{interval_duration.count() / 2};
            std::vector<rpp::schedulers::time_point> item_sent{};
            auto sub = obs.take(3).sample_with_time(sample_period, sample_scheduler).tap([&](const auto&)
            {
                item_sent.push_back(test_scheduler::worker_strategy::now());
            }).subscribe(mock);
            while (sub.is_subscribed())
            {
                interval_scheduler.time_advance(sample_period);
                sample_scheduler.time_advance(rpp::schedulers::duration{}); // empty due to actually test_schedulers use same global time, but each of them drains different queues 
            }

            THEN("emissions happens as soon as item appears")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{0, 1, 2});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);

                CHECK(interval_scheduler.get_schedulings() ==
                      std::vector{start_time + 1 * interval_duration,
                                  start_time + 2 * interval_duration,
                                  start_time + 3 * interval_duration});
                CHECK(sample_scheduler.get_schedulings() ==
                      std::vector{start_time + 1 * sample_period,
                                  start_time + 2 * sample_period,
                                  start_time + 3 * sample_period,
                                  start_time + 4 * sample_period,
                                  start_time + 5 * sample_period,
                                  start_time + 6 * sample_period});

                CHECK(interval_scheduler.get_executions() ==
                      std::vector{start_time + 1 * interval_duration,
                                  start_time + 2 * interval_duration,
                                  start_time + 3 * interval_duration});
                // last item arrived immediately with on_completed BEFORE actually this
                // scheduler does any action
                CHECK(sample_scheduler.get_executions() ==
                      std::vector{start_time + 1 * sample_period,
                                  start_time + 2 * sample_period,
                                  start_time + 3 * sample_period,
                                  start_time + 4 * sample_period,
                                  start_time + 5 * sample_period});
                CHECK(item_sent ==
                      std::vector{start_time + 2 * sample_period,
                                  start_time + 4 * sample_period,
                                  start_time + 6 * sample_period});
            }
        }
        WHEN("subscribe on it via sample with period >= interval period")
        {
            auto sample_period = rpp::schedulers::duration{interval_duration.count() * 2};
            std::vector<rpp::schedulers::time_point> item_sent{};
            auto sub = obs.take(4).sample_with_time(sample_period, sample_scheduler).tap([&](const auto&)
            {
                item_sent.push_back(test_scheduler::worker_strategy::now());
            }).subscribe(mock);
            while (sub.is_subscribed())
            {
                interval_scheduler.time_advance(interval_duration);
                sample_scheduler.time_advance(rpp::schedulers::duration{}); // empty due to actually test_schedulers use same global time, but each of them drains different queues 
            }

            THEN("emissions happens as soon as item appears")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{1, 3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);

                CHECK(interval_scheduler.get_schedulings() ==
                      std::vector{start_time + 1 * interval_duration,
                                  start_time + 2 * interval_duration,
                                  start_time + 3 * interval_duration,
                                  start_time + 4 * interval_duration});
                CHECK(sample_scheduler.get_schedulings() ==
                      std::vector{start_time + 1 * sample_period,
                                  start_time + 2 * sample_period});

                CHECK(interval_scheduler.get_executions() ==
                      std::vector{start_time + 1 * interval_duration,
                                  start_time + 2 * interval_duration,
                                  start_time + 3 * interval_duration,
                                  start_time + 4 * interval_duration});
                // last item arrived immediately with on_completed BEFORE actually this
                // scheduler does any action
                CHECK(sample_scheduler.get_executions() ==
                      std::vector{start_time + 1 * sample_period});
                CHECK(item_sent ==
                      std::vector{start_time + 1 * sample_period,
                                  start_time + 2 * sample_period});
            }
        }
    }
}


SCENARIO("sample sends on_completed immediately", "[operators][sample]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of items")
    {
        auto obs = rpp::source::just(1,2,3);
        WHEN("subscribe on it via sample with infinite like duration")
        {
            auto duration = std::chrono::days{10};
            auto begin = rpp::schedulers::clock_type::now();
            auto sub = obs.sample_with_time(duration, test_scheduler{}).subscribe(mock);
            auto end = rpp::schedulers::clock_type::now();
            THEN("subscriber obtains only item from completed")
            {
                CHECK(end-begin < duration);
                CHECK(!sub.is_subscribed());
                CHECK(mock.get_received_values() == std::vector{3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

SCENARIO("sample sends on_error immediately", "[operators][sample]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of items")
    {
        auto obs = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                     rpp::source::just(1).as_dynamic(),
                                     rpp::source::error<int>(std::exception_ptr{}).as_dynamic()).concat();
        WHEN("subscribe on it via sample with infinite like duration")
        {
            auto duration = std::chrono::days{10};
            auto begin = rpp::schedulers::clock_type::now();
            auto sub = obs.sample_with_time(duration, test_scheduler{}).subscribe(mock);
            auto end = rpp::schedulers::clock_type::now();
            THEN("subscriber obtains only on_error")
            {
                CHECK(end-begin < duration);
                CHECK(!sub.is_subscribed());
                CHECK(mock.get_received_values().empty());
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}