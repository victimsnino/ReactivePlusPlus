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
#include "rpp/schedulers/trampoline_scheduler.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/schedulers/immediate_scheduler.hpp>
#include <rpp/schedulers/new_thread_scheduler.hpp>
#include <rpp/schedulers/run_loop_scheduler.hpp>

#include <future>
#include <sstream>

using namespace std::string_literals;

static std::string get_thread_id_as_string(std::thread::id id = std::this_thread::get_id())
{
    std::stringstream ss;
    ss << id;
    return ss.str();
}

static std::string simulate_nested_scheduling(const auto& worker, std::vector<std::string>& out)
{
    const std::jthread thread([&]
    {
        worker.schedule([&]
        {
            out.push_back("Task 1 starts "s + get_thread_id_as_string());

            worker.schedule([&]
            {
                out.push_back("Task 2 starts "s + get_thread_id_as_string());

                worker.schedule([&]
                {
                    out.push_back("Task 3 runs "s + get_thread_id_as_string());
                    return rpp::schedulers::optional_duration{};
                });

                out.push_back("Task 2 ends "s + get_thread_id_as_string());
                return rpp::schedulers::optional_duration{};
            });

            out.push_back("Task 1 ends "s + get_thread_id_as_string());
            return rpp::schedulers::optional_duration{};
        });
    });

    return get_thread_id_as_string(thread.get_id());
}

SCENARIO("scheduler's worker uses time")
{
    GIVEN("test scheduler")
    {
        auto scheduler = test_scheduler{};
        WHEN("Schedule action without time")
        {
            scheduler.create_worker().schedule([] { return rpp::schedulers::optional_duration{}; });
            THEN("worker obtains schedulable once with current time")
            {
                CHECK(scheduler.get_schedulings() == std::vector{ s_current_time });
            }
        }
        WHEN("Schedule action with some time")
        {
            auto time = rpp::schedulers::clock_type::now();
            scheduler.create_worker().schedule(time, [] { return rpp::schedulers::optional_duration{}; });
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

            auto execution_thread = simulate_nested_scheduling(worker, call_stack);

            THEN("shall see the call-stack in a recursive order")
            {
                REQUIRE(call_stack == std::vector<std::string>{
                        "Task 1 starts "s + execution_thread,
                        "Task 2 starts "s + execution_thread,
                        "Task 3 runs "s + execution_thread,
                        "Task 2 ends "s + execution_thread,
                        "Task 1 ends "s + execution_thread,
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

SCENARIO("trampoline scheduler dispatches task in the same thread")
{
    auto scheduler = rpp::schedulers::trampoline{};
    auto sub       = rpp::composite_subscription{};
    auto worker    = scheduler.create_worker(sub);

    WHEN("supply a simple job")
    {
        std::optional<std::thread::id> observed_thread_id;

        worker.schedule([&]() -> rpp::schedulers::optional_duration
                        {
                            observed_thread_id = std::this_thread::get_id();
                            return std::nullopt;
                        });

        THEN("thread ID shall match with the current thread ID")
        {
            REQUIRE(observed_thread_id.value() == std::this_thread::get_id());
        }
    }

    WHEN("supply a job that schedules inner job")
    {
        std::optional<std::thread::id> observed_thread_id;

        worker.schedule(
            [&]() -> rpp::schedulers::optional_duration
            {
                worker.schedule(
                    [&]() ->rpp::schedulers::optional_duration
                    {
                        observed_thread_id = std::this_thread::get_id();
                        return std::nullopt;
                    });
                return std::nullopt;
            });

        THEN("thread ID from the inner schedulable shall match with the current thread ID")
        {
            REQUIRE(observed_thread_id.value() == std::this_thread::get_id());
        }
    }
}

static std::vector<std::string> trampoline_expected_simulate_nested_scheduling(std::string thread_id)
{
    return std::vector<std::string>{
        "Task 1 starts "s + thread_id,
        "Task 1 ends "s + thread_id,
        "Task 2 starts "s + thread_id,
        "Task 2 ends "s + thread_id,
        "Task 3 runs "s + thread_id,
    };
}
SCENARIO("trampoline scheduler defers tasks in order")
{
    auto scheduler = rpp::schedulers::trampoline{};
    auto sub       = rpp::composite_subscription{};
    auto worker    = scheduler.create_worker(sub);
    rpp::subscription_guard guard{sub};

    WHEN("supply a job that schedules inner job that schedules inner job")
    {
        std::vector<std::string> call_stack;

        auto execution_thread = simulate_nested_scheduling(worker, call_stack);

        THEN("order of call-stack must be in order")
        {
            REQUIRE(call_stack == trampoline_expected_simulate_nested_scheduling(execution_thread));
        }
    }
}

SCENARIO("trampoline scheduler is thread local")
{
    WHEN("two threads are using the same trampoline scheduler")
    {
        auto scheduler = rpp::schedulers::trampoline{};
        auto sub       = rpp::composite_subscription{};
        auto worker    = scheduler.create_worker(sub);
        rpp::subscription_guard guard{sub};

        std::vector<std::string> call_stack_1;
        std::vector<std::string> call_stack_2;

        auto thread_id_1 = simulate_nested_scheduling(worker, call_stack_1);
        auto thread_id_2 = simulate_nested_scheduling(worker, call_stack_2);

        THEN("call stack of two threads shall be separate")
        {
            REQUIRE(call_stack_1 == trampoline_expected_simulate_nested_scheduling(thread_id_1));
            REQUIRE(call_stack_2 == trampoline_expected_simulate_nested_scheduling(thread_id_2));
        }
    }
}

SCENARIO("trampoline scheduler regards unsubscribed subscription")
{
    GIVEN("unsubscribed subscription")
    {
        auto scheduler = rpp::schedulers::trampoline{};
        auto sub       = rpp::composite_subscription{};
        auto worker    = scheduler.create_worker(sub);

        sub.unsubscribe();

        THEN("shall not see execution of schedulable")
        {
            worker.schedule([&]() -> rpp::schedulers::optional_duration
            {
                REQUIRE(false);
                return std::nullopt;
            });
        }
    }

    GIVEN("asynchronously unsubscribes subscription")
    {
        auto scheduler = rpp::schedulers::trampoline{};
        auto sub       = rpp::composite_subscription{};
        auto worker    = scheduler.create_worker(sub);

        THEN("shall not see execution of inner schedulable")
        {
            worker.schedule([&]() -> rpp::schedulers::optional_duration
            {
                worker.schedule([&]() -> rpp::schedulers::optional_duration
                {
                    REQUIRE(false);
                    return std::nullopt;
                });

                sub.unsubscribe();
                return std::nullopt;
            });
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
