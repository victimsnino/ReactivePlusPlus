//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "copy_count_tracker.hpp"
#include "mock_observer.hpp"
#include "test_scheduler.hpp"

#include <catch2/catch_test_macros.hpp>

#include <rpp/schedulers.hpp>
#include <rpp/subjects/publish_subject.hpp>
#include <rpp/operators/observe_on.hpp>
#include <rpp/sources.hpp>
#include <set>


SCENARIO("observe_on transfers emssions to scheduler", "[operators][observe_on]")
{
    auto mock = mock_observer<std::string>{};
    auto scheduler = test_scheduler{};
    auto initial_time  = test_scheduler::worker_strategy::now();

    GIVEN("observable with item")
    {
        auto vals = std::vector<std::string>{"2", "3"};
        auto obs  = rpp::source::from_iterable(vals);

        WHEN("subscribe on observable via observe_on with test scheduler")
        {
            auto res = obs.observe_on(scheduler);
            THEN("obtains values in the same order via scheduling")
            {
                res.subscribe(mock);

                CHECK(mock.get_received_values() == vals);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(scheduler.get_schedulings() == std::vector{ initial_time, initial_time, initial_time });//2 items + on_completed
                CHECK(scheduler.get_executions() == std::vector{ initial_time, initial_time, initial_time });//2 items + on_completed
            }
        }
    }

    GIVEN("observable with error")
    {
        auto obs = rpp::source::error<std::string>(std::make_exception_ptr(std::runtime_error{""}));

        WHEN("subscribe on observable via observe_on with test scheduler")
        {
            auto res = obs.observe_on(scheduler);
            THEN("obtain error with scheduling")
            {
                res.subscribe(mock);

                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(scheduler.get_schedulings() == std::vector{ initial_time });
                CHECK(scheduler.get_executions() == std::vector{ initial_time });

            }
        }
    }

    GIVEN("observable with completed")
    {
        auto obs = rpp::source::empty<std::string>();

        WHEN("subscribe on observable via observe_on with test scheduler")
        {
            auto res = obs.observe_on(scheduler);
            THEN("obtain error with scheduling")
            {
                res.subscribe(mock);

                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(scheduler.get_schedulings() == std::vector{ initial_time });
                CHECK(scheduler.get_executions() == std::vector{ initial_time });

            }
        }
    }

    GIVEN("subject with items")
    {
        auto int_mock = mock_observer<int>{};
        auto subj = rpp::subjects::publish_subject<int>{};
        WHEN("subscribe on subject via observe_on and doing recursive submit")
        {
            auto sub = subj.get_observable()
                .observe_on(scheduler)
                .subscribe([&](int v)
                {
                    int_mock.on_next(v);

                    if (v == 1)
                    {
                        subj.get_subscriber().on_next(2);
                        THEN("no direct schedule to scheduler after recursive on_next")
                        {
                            CHECK(scheduler.get_schedulings() == std::vector{ initial_time });
                            CHECK(scheduler.get_executions()  == std::vector{ initial_time });
                            CHECK(int_mock.get_received_values() == std::vector{1});
                        }
                    }
                });

            subj.get_subscriber().on_next(1);

            THEN("second job executed without extra schedule")
            {
                CHECK(scheduler.get_schedulings() == std::vector{ initial_time });
                CHECK(scheduler.get_executions()  == std::vector{ initial_time });
                CHECK(int_mock.get_received_values() == std::vector{ 1, 2 });
            }
        }

        WHEN("subscribe on subject via observe_on trampoline and doing recursive submit from another thread")
        {
            THEN("all values obtained in the same thread")
            {
                auto current_thread = std::this_thread::get_id();

                auto sub = subj.get_observable()
                    .observe_on(rpp::schedulers::trampoline{})
                    .subscribe([&](int v)
                    {
                        CHECK(std::this_thread::get_id() == current_thread);

                        int_mock.on_next(v);

                        if (v == 1)
                        {
                            std::thread{[&]{subj.get_subscriber().on_next(2);}}.join();

                            THEN("no recursive on_next calls")
                            {
                                CHECK(int_mock.get_received_values() == std::vector{1});
                            }
                        }
                    });

                subj.get_subscriber().on_next(1);

                AND_THEN("all values obtained")
                {
                    CHECK(int_mock.get_received_values() == std::vector{ 1, 2 });
                }
            }
        }
    }
}