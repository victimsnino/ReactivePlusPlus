//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <mock_observer.hpp>
#include <test_scheduler.hpp>
#include <catch2/catch_test_macros.hpp>
#include <rpp/operators/debounce.hpp>
#include <rpp/subjects/publish_subject.hpp>

SCENARIO("debounce emit only items where timeout reached", "[operators][debounce]")
{
    auto           debounce_delay = std::chrono::seconds{2};
    test_scheduler scheduler{};
    auto           start = s_current_time;

    GIVEN("subject of items and subscriber subscribed on it via debounce")
    {
        auto mock = mock_observer<int>{};
        auto subj = rpp::subjects::publish_subject<int>{};
        subj.get_observable().debounce(debounce_delay, scheduler).subscribe(mock);
        WHEN("emit value")
        {
            subj.get_subscriber().on_next(1);
            THEN("delay scheduled action to track period")
            {
                CHECK(scheduler.get_schedulings() == std::vector{start+debounce_delay});
                CHECK(scheduler.get_executions().empty());
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
            AND_WHEN("scheduler reached delayed time")
            {
                scheduler.time_advance(debounce_delay);
                THEN("emission reached mock")
                {
                    CHECK(scheduler.get_schedulings() == std::vector{start+debounce_delay});
                    CHECK(scheduler.get_executions() == std::vector{start+debounce_delay});
                    CHECK(mock.get_received_values() == std::vector{1});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
            AND_WHEN("emit on completed")
            {
                subj.get_subscriber().on_completed();
                THEN("emission reached mock with on completed without schedulable exection")
                {
                    CHECK(scheduler.get_schedulings() == std::vector{start+debounce_delay});
                    CHECK(scheduler.get_executions().empty());
                    CHECK(mock.get_received_values() == std::vector{1});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
            }
            AND_WHEN("new value emitted before scheduler reached requested time")
            {
                scheduler.time_advance(debounce_delay / 2);
                subj.get_subscriber().on_next(2);
                THEN("nothing changed immediately")
                {
                    CHECK(scheduler.get_schedulings() == std::vector{start+debounce_delay});
                    CHECK(scheduler.get_executions().empty());
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
                AND_WHEN("scheduler reached originally requested time")
                {
                    scheduler.time_advance(debounce_delay / 2);
                    THEN("delay re-schedule schedulable to new delay timepoint")
                    {
                        CHECK(scheduler.get_schedulings() == std::vector{start+debounce_delay, start+debounce_delay/2+debounce_delay});
                        CHECK(scheduler.get_executions() == std::vector{start+debounce_delay});
                        CHECK(mock.get_total_on_next_count() == 0);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                    }
                    AND_WHEN("scheduler reached delayed time")
                    {
                        scheduler.time_advance(debounce_delay/2);
                        THEN("emission reached mock")
                        {
                            CHECK(scheduler.get_schedulings() == std::vector{start+debounce_delay, start+debounce_delay/2+debounce_delay});
                            CHECK(scheduler.get_executions() == std::vector{start+debounce_delay, start+debounce_delay/2+debounce_delay});
                            CHECK(mock.get_received_values() == std::vector{2});
                            CHECK(mock.get_on_error_count() == 0);
                            CHECK(mock.get_on_completed_count() == 0);
                        }
                    }
                }
            }
        }
    }
}
