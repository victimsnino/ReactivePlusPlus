//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "test_scheduler.hpp"
#include "rpp/subscriptions/composite_subscription.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/schedulers/immediate_scheduler.hpp>
#include <rpp/schedulers/new_thread_scheduler.hpp>
#include <rpp/schedulers/run_loop_scheduler.hpp>

#include <future>

SCENARIO("scheduler's worker uses time")
{
    GIVEN("test scheduler")
    {
        auto scheduler = test_scheduler{};
        WHEN("Schedule action without time")
        {
            scheduler.create_worker().schedule([]() { return rpp::schedulers::optional_duration{}; });
            THEN("worker obtains schedulable once with current time")
            {
                CHECK(scheduler.get_schedulings() == std::vector{ s_current_time });
            }
        }
        WHEN("Schedule action with some time")
        {
            auto time = rpp::schedulers::clock_type::now();
            scheduler.create_worker().schedule(time, []() { return rpp::schedulers::optional_duration{}; });
            THEN("worker obtains schedulable once with provided time")
            {
                CHECK(scheduler.get_schedulings() == std::vector{ time });
            }
        }
        WHEN("Schedule action with zero duration")
        {
            std::vector<rpp::schedulers::time_point> schedulings{};
            scheduler.create_worker().schedule([&]() -> rpp::schedulers::optional_duration
                {
                    schedulings.push_back(s_current_time);
                    s_current_time += std::chrono::seconds{1};
                    if (schedulings.size() == 3)
                        return std::nullopt;
                    return rpp::schedulers::duration{};
                });
            THEN("worker obtains schedulable three times with current time and time+diff")
            {
                CHECK(scheduler.get_schedulings() == schedulings);
            }
        }
        WHEN("Schedule action with repeat")
        {
            int                                 count = 0;
            constexpr rpp::schedulers::duration dur = std::chrono::seconds{ 3 };
            scheduler.create_worker().schedule([&]() -> rpp::schedulers::optional_duration
                {
                    if (++count > 2)
                        return std::nullopt;
                    return dur;
                });
            THEN("worker obtains schedulable three times with current time and time+diff")
            {
                CHECK(scheduler.get_schedulings() == std::vector{ s_current_time, s_current_time + dur , s_current_time + dur + dur });
            }
        }
    }
}

SCENARIO("Immediate scheduler schedules task immediately")
{
    GIVEN("immediate_scheduler")
    {
        auto scheduler = rpp::schedulers::immediate{};

        auto   worker = scheduler.create_worker();
        size_t call_count{};

        WHEN("scheduler without time and resheduling")
        {
            THEN("called once immediately")
            {
                worker.schedule([&call_count]() -> rpp::schedulers::optional_duration
                {
                    ++call_count;
                    return {};
                });
                CHECK(call_count == 1);
            }
        }
        WHEN("scheduler without time with scheduling")
        {
            THEN("called twice immediately")
            {
                worker.schedule([&call_count]() -> rpp::schedulers::optional_duration
                {
                    if (++call_count <= 1)
                        return rpp::schedulers::duration{};
                    return {};
                });

                CHECK(call_count == 2);
            }
        }
        WHEN("scheduler with time")
        {
            THEN("called twice immediately")
            {
                auto now  = rpp::schedulers::clock_type::now();
                auto diff = std::chrono::seconds{1};

                rpp::schedulers::time_point execute_time{};
                worker.schedule(now + diff,
                                 [&call_count, &execute_time]() -> rpp::schedulers::optional_duration
                                 {
                                     ++call_count;
                                     execute_time = rpp::schedulers::clock_type::now();
                                     return {};
                                 });

                REQUIRE(call_count == 1);
                REQUIRE(execute_time - now >= diff);
            }
        }
        WHEN("reshedule from function")
        {
            THEN("called twice immediately")
            {
                std::vector<rpp::schedulers::time_point> executions{};
                std::chrono::milliseconds                diff = std::chrono::seconds{1};

                worker.schedule([&call_count, &executions, &diff]() -> rpp::schedulers::optional_duration
                {
                    executions.push_back(rpp::schedulers::clock_type::now());
                    if (++call_count <= 1)
                        return diff;
                    return {};
                });

                REQUIRE(call_count == 2);
                REQUIRE(executions[1] - executions[0] >= (diff - std::chrono::milliseconds(500)));
            }
        }
        WHEN("schedule a task that schedule a task that schedule a task")
        {
            std::vector<std::string> call_stack;

            worker.schedule([&]() -> rpp::schedulers::optional_duration
            {
                call_stack.emplace_back("task 1 starts");
                worker.schedule([&]() -> rpp::schedulers::optional_duration
                {
                    call_stack.emplace_back("task 2 starts");
                    worker.schedule([&]() -> rpp::schedulers::optional_duration
                    {
                        call_stack.emplace_back("task 3 runs");
                        return std::nullopt;
                    });
                    call_stack.emplace_back("task 2 ends");
                    return std::nullopt;
                });
                call_stack.emplace_back("task 1 ends");
                return std::nullopt;
            });

            THEN("shall see the call-stack in a specific order")
            {
                REQUIRE(call_stack == std::vector<std::string>{
                    "task 1 starts",
                    "task 2 starts",
                    "task 3 runs",
                    "task 2 ends",
                    "task 1 ends",
                });
            }
        }
    }
}

SCENARIO("Immediate scheduler depends on subscription")
{
    GIVEN("immediate_scheduler")
    {
        auto                        scheduler = rpp::schedulers::immediate{};
        rpp::composite_subscription sub{};
        auto                        worker = scheduler.create_worker(sub);

        size_t call_count{};

        WHEN("pass unsubscribed subscription")
        {
            THEN("no any calls/schedules")
            {
                sub.unsubscribe();

                worker.schedule([&call_count]() -> rpp::schedulers::optional_duration
                {
                    ++call_count;
                    return rpp::schedulers::duration{};
                });

                CHECK(call_count == 0);
            }
        }
        WHEN("unsubscribe during function")
        {
            THEN("no any calls/schedules after unsubscribe")
            {
                worker.schedule([&call_count, sub]() -> rpp::schedulers::optional_duration
                {
                    if (++call_count > 1)
                        sub.unsubscribe();
                    return rpp::schedulers::duration{};
                });

                CHECK(call_count == 2);
            }
        }
    }
}

SCENARIO("New thread scheduler schedules tasks into separate thread")
{
    GIVEN("NewThread scheduler")
    {
        auto scheduler = rpp::schedulers::new_thread{};
        auto sub = rpp::composite_subscription{};
        auto worker    = scheduler.create_worker(sub);
        rpp::subscription_guard guard{sub};
        WHEN("schedules job to worker")
        {
            THEN("job executed in another thread")
            {
                std::promise<std::thread::id> promise{};
                auto                          future = promise.get_future();
                worker.schedule([&]() -> rpp::schedulers::optional_duration
                {
                    promise.set_value(std::this_thread::get_id());
                    return {};
                });

                REQUIRE(future.wait_for(std::chrono::seconds{5})==std::future_status::ready);

                REQUIRE(future.valid());
                REQUIRE(future.get() != std::this_thread::get_id());
            }
        }
        WHEN("schedules jobs to worker with some delay")
        {
            THEN("first job executed later")
            {
                std::promise<rpp::schedulers::time_point> promise_1{};
                std::promise<rpp::schedulers::time_point> promise_2{};
                auto future_1 = promise_1.get_future();
                auto future_2 = promise_2.get_future();
                auto now = rpp::schedulers::clock_type::now();
                auto set_promise = [](std::promise<rpp::schedulers::time_point>& promise)
                {
                    return [&]() -> rpp::schedulers::optional_duration
                    {
                        promise.set_value(rpp::schedulers::clock_type::now());
                        return {};
                    };
                };

                worker.schedule(now + std::chrono::seconds{2}, set_promise(promise_2));
                worker.schedule(now + std::chrono::seconds{1}, set_promise(promise_1));

                REQUIRE(future_1.wait_for(std::chrono::seconds{10})==std::future_status::ready);
                REQUIRE(future_2.wait_for(std::chrono::seconds{10})==std::future_status::ready);

                REQUIRE(future_1.valid());
                REQUIRE(future_2.valid());
                REQUIRE(future_1.get() < future_2.get());
            }
        }
        WHEN("reshedule from function")
        {
            THEN("called twice immediately")
            {
                std::vector<rpp::schedulers::time_point> executions{};
                std::chrono::milliseconds                diff = std::chrono::seconds{1};
                size_t                                   call_count{};
                worker.schedule([&call_count, &executions, &diff]() -> rpp::schedulers::optional_duration
                {
                    executions.push_back(rpp::schedulers::clock_type::now());
                    if (++call_count <= 1)
                        return diff;
                    return {};
                });

                std::this_thread::sleep_for(diff * 2);

                REQUIRE(call_count == 2);
                REQUIRE(executions[1] - executions[0] >= (diff-std::chrono::milliseconds(500))); 
            }
        }
    }
}

SCENARIO("New thread scheduler depends on subscription")
{
    GIVEN("NewThread scheduler")
    {
        auto                        scheduler = rpp::schedulers::new_thread{};
        rpp::composite_subscription sub{};
        auto                        worker = scheduler.create_worker(sub);
        rpp::subscription_guard     guard{sub};


        size_t call_count{};

        WHEN("pass unsubscribed subscription")
        {
            THEN("no any calls/schedules")
            {
                sub.unsubscribe();

                worker.schedule([&call_count]() -> rpp::schedulers::optional_duration
                    {
                        ++call_count;
                        return rpp::schedulers::duration{};
                    });
                std::this_thread::sleep_for(std::chrono::seconds{ 1 });
                CHECK(call_count == 0);
            }
        }
        WHEN("unsubscribe during function")
        {
            THEN("no any calls/schedules after unsubscribe")
            {
                worker.schedule([&call_count, sub]() -> rpp::schedulers::optional_duration
                    {
                        if (++call_count > 1)
                            sub.unsubscribe();
                        return rpp::schedulers::duration{};
                    });

                std::this_thread::sleep_for(std::chrono::seconds{ 1 });
                CHECK(call_count == 2);
            }
        }
        WHEN("unsubscribe before time")
        {
            THEN("no any calls/schedules after unsubscribe")
            {
                std::promise<bool> called{};
                auto               future = called.get_future();
                auto               diff   = std::chrono::seconds{2};
                worker.schedule(rpp::schedulers::clock_type::now() + diff,
                                 [&called]() -> rpp::schedulers::optional_duration
                                 {
                                     called.set_value(true);
                                     return rpp::schedulers::duration{};
                                 });
                sub.unsubscribe();
                REQUIRE(future.wait_for(diff)==std::future_status::timeout);
                CHECK(future.valid());
            }
        }
        WHEN("unsubscribe before time multiple")
        {
            THEN("no any calls/schedules after unsubscribe")
            {
                std::promise<bool> called{};
                auto               future = called.get_future();
                auto               diff = std::chrono::seconds{ 2 };
                for (size_t i = 0; i < 3; ++i)
                {
                    worker.schedule(rpp::schedulers::clock_type::now() + diff,
                        [&called]() -> rpp::schedulers::optional_duration
                        {
                            called.set_value(true);
                            return rpp::schedulers::duration{};
                        });
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                sub.unsubscribe();
                REQUIRE(future.wait_for(diff)==std::future_status::timeout);
                CHECK(future.valid());
            }
        }
    }
}

SCENARIO("RunLoop scheduler dispatches tasks only manually")
{
    GIVEN("run loop scheduler")
    {
        auto scheduler = rpp::schedulers::run_loop{};
        WHEN("submit work to it")
        {
            size_t schedulable_1_executed_count{};
            size_t schedulable_2_executed_count{};
            scheduler.create_worker().schedule([&]() -> rpp::schedulers::optional_duration {++schedulable_1_executed_count; return {}; });
            scheduler.create_worker().schedule([&]() -> rpp::schedulers::optional_duration {++schedulable_2_executed_count; return {}; });
            THEN("nothing happens but scheduler has schedulable to dispatch")
            {
                CHECK(schedulable_1_executed_count == 0);
                CHECK(schedulable_2_executed_count == 0);
                CHECK(scheduler.is_empty() == false);
                CHECK(scheduler.is_any_ready_schedulable() == true);
            }
            AND_WHEN("call dispatch")
            {
                scheduler.dispatch_if_ready();
                THEN("only first schedulable dispatched")
                {
                    CHECK(schedulable_1_executed_count == 1);
                    CHECK(schedulable_2_executed_count == 0);
                    CHECK(scheduler.is_empty() == false);
                    CHECK(scheduler.is_any_ready_schedulable() == true);
                }
                AND_WHEN("call dispatch again")
                {
                    scheduler.dispatch_if_ready();
                    THEN("both schedulable dispatched")
                    {
                        CHECK(schedulable_1_executed_count == 1);
                        CHECK(schedulable_2_executed_count == 1);
                        CHECK(scheduler.is_empty() == true);
                        CHECK(scheduler.is_any_ready_schedulable() == false);
                    }
                }
            }
        }
    }
}
