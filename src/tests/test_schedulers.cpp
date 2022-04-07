// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "rpp/subscriptions/composite_subscription.h"

#include <catch2/catch_test_macros.hpp>
#include <rpp/schedulers/immediate_scheduler.h>
#include <rpp/schedulers/new_thread_scheduler.h>

#include <future>

SCENARIO("Immediate scheduler schedule task immediately")
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

                REQUIRE(future_1.wait_for(std::chrono::seconds{2})==std::future_status::ready);
                REQUIRE(future_2.wait_for(std::chrono::seconds{2})==std::future_status::ready);

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
                auto               diff   = std::chrono::seconds{5};
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
                auto               diff = std::chrono::seconds{ 5 };
                for (size_t i = 0; i < 5; ++i)
                {
                    worker.schedule(rpp::schedulers::clock_type::now() + diff,
                        [&called]() -> rpp::schedulers::optional_duration
                        {
                            called.set_value(true);
                            return rpp::schedulers::duration{};
                        });
                }
                sub.unsubscribe();
                REQUIRE(future.wait_for(diff)==std::future_status::timeout);
                CHECK(future.valid());
            }
        }
    }
}
