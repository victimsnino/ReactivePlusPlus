//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "mock_observer.hpp"
#include "rpp/disposables/fwd.hpp"

#include <rpp/disposables/callback_disposable.hpp>
#include <snitch/snitch.hpp>
#include <rpp/schedulers.hpp>
#include <rpp/observers/lambda_observer.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/operators/as_blocking.hpp>
#include <rpp/operators/subscribe_on.hpp>

#include <chrono>
#include <future>
#include <optional>
#include <sstream>
#include <string>
#include <stdexcept>
#include <thread>

using namespace std::string_literals;

static std::string get_thread_id_as_string(std::thread::id id = std::this_thread::get_id())
{
    std::stringstream ss;
    ss << id;
    return ss.str();
}

static std::string simulate_nested_scheduling(auto worker, const auto& obs, std::vector<std::string>& out)
{
    std::thread thread([&, worker]
    {
        worker.schedule([&, worker](const auto&)
        {
            out.push_back("Task 1 starts "s + get_thread_id_as_string());

            worker.schedule([&, worker](const auto&)
            {
                out.push_back("Task 2 starts "s + get_thread_id_as_string());

                worker.schedule([&](const auto&)
                {
                    out.push_back("Task 3 runs "s + get_thread_id_as_string());
                    return rpp::schedulers::optional_duration{};
                }, obs);

                out.push_back("Task 2 ends "s + get_thread_id_as_string());
                return rpp::schedulers::optional_duration{};
            }, obs);

            out.push_back("Task 1 ends "s + get_thread_id_as_string());
            return rpp::schedulers::optional_duration{};
        }, obs);
    });

    auto threadid = get_thread_id_as_string(thread.get_id());
    thread.join();
    return threadid;
}

static std::string simulate_complex_scheduling(const auto& worker, const auto& obs, std::vector<std::string>& out)
{
    std::thread thread([&, worker]
    {
        worker.schedule([&, worker](const auto&)
        {
            out.push_back("Task 1 starts "s + get_thread_id_as_string());

            worker.schedule([&, worker](const auto&, int& counter) -> rpp::schedulers::optional_duration
            {
                out.push_back("Task 2 starts "s + get_thread_id_as_string());

                worker.schedule([&](const auto&)
                {
                    out.push_back("Task 4 runs "s + get_thread_id_as_string());
                    return rpp::schedulers::optional_duration{};
                }, obs);

                out.push_back("Task 2 ends "s + get_thread_id_as_string());
                if (counter++ < 1)
                    return std::chrono::nanoseconds{1};
                return std::nullopt;
            }, obs, int{});

            worker.schedule([&](const auto&, int& counter) -> rpp::schedulers::optional_duration
            {
                out.push_back("Task 3 starts "s + get_thread_id_as_string());

                out.push_back("Task 3 ends "s + get_thread_id_as_string());
                if (counter++ < 1)
                    return std::chrono::nanoseconds{1};
                return std::nullopt;
            }, obs, int{});

            out.push_back("Task 1 ends "s + get_thread_id_as_string());
            return rpp::schedulers::optional_duration{};
        }, obs);
    });

    auto threadid = get_thread_id_as_string(thread.get_id());
    thread.join();
    return threadid;
}

static std::string simulate_complex_scheduling_with_delay(const auto& worker, const auto& obs, std::vector<std::string>& out)
{
    std::thread thread([&, worker]
    {
        worker.schedule([&, worker](const auto&)
        {
            out.push_back("Task 1 starts "s + get_thread_id_as_string());

            worker.schedule([&, worker](const auto&, int& counter) -> rpp::schedulers::optional_duration
            {
                out.push_back("Task 2 starts "s + get_thread_id_as_string());

                worker.schedule(std::chrono::milliseconds{50}, [&](const auto&)
                {
                    out.push_back("Task 4 runs "s + get_thread_id_as_string());
                    return rpp::schedulers::optional_duration{};
                }, obs);

                out.push_back("Task 2 ends "s + get_thread_id_as_string());
                if (counter++ < 1)
                    return std::chrono::nanoseconds{1};
                return std::nullopt;
            }, obs, int{});

            worker.schedule([&](const auto&, int& counter) -> rpp::schedulers::optional_duration
            {
                out.push_back("Task 3 starts "s + get_thread_id_as_string());

                out.push_back("Task 3 ends "s + get_thread_id_as_string());
                if (counter++ < 1)
                    return std::chrono::nanoseconds{1};
                return std::nullopt;
            }, obs, int{});

            out.push_back("Task 1 ends "s + get_thread_id_as_string());
            return rpp::schedulers::optional_duration{};
        }, obs);
    });

    auto threadid = get_thread_id_as_string(thread.get_id());
    thread.join();
    return threadid;
}

TEST_CASE("Immediate scheduler")
{
    auto scheduler = rpp::schedulers::immediate{};
    auto d = rpp::composite_disposable_wrapper{std::make_shared<rpp::composite_disposable>()};
    auto mock_obs = mock_observer_strategy<int>{};
    auto obs = mock_obs.get_observer(d).as_dynamic();

    auto worker = scheduler.create_worker();

    CHECK(worker.get_disposable().is_disposed());

    size_t call_count{};

    SECTION("immediate scheduler schedules and re-schedules action immediately")
    {
        worker.schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
        {
            if (++call_count <= 1)
                return std::chrono::nanoseconds{1};
            return {};
        }, obs);

        CHECK(call_count == 2);
    }

    SECTION("immediate scheduler schedules action with delay")
    {
        auto now  = rpp::schedulers::clock_type::now();
        auto diff = std::chrono::milliseconds{500};

        rpp::schedulers::time_point execute_time{};
        worker.schedule(diff,
                        [&call_count, &execute_time](const auto&) -> rpp::schedulers::optional_duration
                        {
                            ++call_count;
                            execute_time = rpp::schedulers::clock_type::now();
                            return {};
                        }, obs);
        REQUIRE(call_count == 1);
        REQUIRE(execute_time - now >= diff);
    }

    SECTION("immediate scheduler re-schedules action at provided timepoint")
    {
        std::vector<rpp::schedulers::time_point> executions{};
        std::chrono::milliseconds                diff = std::chrono::milliseconds{500};
        worker.schedule([&call_count,&executions, &diff](const auto&) -> rpp::schedulers::optional_duration
                        {
                            executions.push_back(rpp::schedulers::clock_type::now());
                            if (++call_count <= 1)
                                return diff;
                            return {};
                        }, obs);

        REQUIRE(call_count == 2);
        REQUIRE(executions[1] - executions[0] >= (diff - std::chrono::milliseconds(100)));
    }

    SECTION("immediate scheduler with nesting scheduling should be like call-stack in a recursive order")
    {
        std::vector<std::string> call_stack;

            auto execution_thread = simulate_nested_scheduling(worker, obs, call_stack);

            REQUIRE(call_stack == std::vector<std::string>{
                    "Task 1 starts "s + execution_thread,
                    "Task 2 starts "s + execution_thread,
                    "Task 3 runs "s + execution_thread,
                    "Task 2 ends "s + execution_thread,
                    "Task 1 ends "s + execution_thread,
                    });
    }

    SECTION("immediate scheduler with complex scheduling with delay should be like call-stack in a recursive order")
    {
        std::vector<std::string> call_stack;

            auto execution_thread = simulate_complex_scheduling_with_delay(worker, obs, call_stack);

            REQUIRE(call_stack == std::vector<std::string>{
                    "Task 1 starts "s + execution_thread,
                    "Task 2 starts "s + execution_thread,
                    "Task 4 runs "s + execution_thread,
                    "Task 2 ends "s + execution_thread,
                    "Task 2 starts "s + execution_thread,
                    "Task 4 runs "s + execution_thread,
                    "Task 2 ends "s + execution_thread,
                    "Task 3 starts "s + execution_thread,
                    "Task 3 ends "s + execution_thread,
                    "Task 3 starts "s + execution_thread,
                    "Task 3 ends "s + execution_thread,
                    "Task 1 ends "s + execution_thread,
            });
    }
    SECTION("immediate scheduler with complex scheduling should be like call-stack in a recursive order")
    {
        std::vector<std::string> call_stack;

            auto execution_thread = simulate_complex_scheduling(worker, obs, call_stack);

            REQUIRE(call_stack == std::vector<std::string>{
                    "Task 1 starts "s + execution_thread,
                    "Task 2 starts "s + execution_thread,
                    "Task 4 runs "s + execution_thread,
                    "Task 2 ends "s + execution_thread,
                    "Task 2 starts "s + execution_thread,
                    "Task 4 runs "s + execution_thread,
                    "Task 2 ends "s + execution_thread,
                    "Task 3 starts "s + execution_thread,
                    "Task 3 ends "s + execution_thread,
                    "Task 3 starts "s + execution_thread,
                    "Task 3 ends "s + execution_thread,
                    "Task 1 ends "s + execution_thread,
            });
    }

    SECTION("immediate scheduler does nothing with disposed observer")
    {
        d.dispose();
        worker.schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
        {
            ++call_count;
            return std::chrono::nanoseconds{1};
        }, obs);

        CHECK(call_count == 0);
    }

    SECTION("immediate scheduler does nothing with observer disposed during wait")
    {
        worker.schedule(
            [&call_count, obs](const auto&) -> rpp::schedulers::optional_duration
            {
                ++call_count;
                std::thread{[obs]()
                            {
                                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                obs.on_completed();
                            }}
                    .detach();
                return std::chrono::milliseconds{200};
            },
            obs);

        CHECK(call_count == 1);
    }

    SECTION("immediate scheduler does not reschedule after disposing inside schedulable")
    {
        worker.schedule([&call_count, &d](const auto&) -> rpp::schedulers::optional_duration
        {
            if (++call_count > 1)
                d.dispose();
            return std::chrono::nanoseconds{1};
        }, obs);

        CHECK(call_count == 2);
    }

    SECTION("immediate scheduler forwards any arguments")
    {
        worker.schedule([](const auto&, int, const std::string&){ return rpp::schedulers::optional_duration{}; }, obs, int{}, std::string{});
    }

    SECTION("error during schedulable")
    {
        worker.schedule([](const auto&) -> rpp::schedulers::optional_duration {throw std::runtime_error{"test"};}, obs);
        CHECK(mock_obs.get_on_error_count()  == 1);
    }
}

TEMPLATE_TEST_CASE("queue_based scheduler", "", rpp::schedulers::current_thread, rpp::schedulers::new_thread)
{
    auto d = std::make_shared<rpp::composite_disposable>();
    auto mock_obs = mock_observer_strategy<int>{};
    auto obs = std::optional{mock_obs.get_observer(d).as_dynamic()};

    auto worker = std::optional{TestType::create_worker()};
    if constexpr (std::same_as<TestType, rpp::schedulers::current_thread>)
        CHECK(worker->get_disposable().is_disposed());

    obs->set_upstream(worker->get_disposable());
    size_t call_count{};

    std::promise<std::string> thread_of_schedule_promise{};

    auto done = std::make_shared<std::atomic_bool>();

    worker->schedule([&](const auto&)
    {
        thread_of_schedule_promise.set_value(get_thread_id_as_string(std::this_thread::get_id()));
        if constexpr (std::same_as<TestType, rpp::schedulers::new_thread>)
            thread_local rpp::utils::finally_action a{[done] { done->store(true); }};
        else
            done->store(true);

        return rpp::schedulers::optional_duration{};
    }, obs.value());

    auto thread_of_execution = thread_of_schedule_promise.get_future().get();

    auto get_thread = [&]([[maybe_unused]] const std::string& thread_of_schedule)
    {
        if constexpr (std::same_as<TestType, rpp::schedulers::current_thread>)
            return thread_of_schedule;
        else
            return thread_of_execution;
    };

    auto wait_till_finished = [&]
    {
        worker.reset();
        obs.reset();
        d.reset();

        while(!done->load()){};
    };

    SECTION("scheduler schedules and re-schedules action immediately")
    {
        worker->schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
        {
            if (++call_count <= 1)
                return std::chrono::nanoseconds{1};
            return std::nullopt;
        }, obs.value());

        wait_till_finished();

        CHECK(call_count == 2);
    }

    SECTION("scheduler recursive scheduling")
    {
        worker->schedule([&call_count, worker](const auto& obs) -> rpp::schedulers::optional_duration
        {
            worker->schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
            {
                if (++call_count <= 1)
                    return std::chrono::nanoseconds{1};
                return std::nullopt;
            }, obs);
            return std::nullopt;
        }, obs.value());

        wait_till_finished();

        CHECK(call_count == 2);
    }

    SECTION("scheduler recursive scheduling with original")
    {
        worker->schedule([&call_count, worker](const auto& obs) -> rpp::schedulers::optional_duration
        {
            worker->schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
            {
                if (++call_count <= 1)
                    return std::chrono::nanoseconds{1};
                return std::nullopt;
            }, obs);

            if (call_count == 0)
                return std::chrono::nanoseconds{1};
            return std::nullopt;
        }, obs.value());

        wait_till_finished();

        CHECK(call_count == 3);
    }

    SECTION("scheduler schedules action with delay")
    {
        auto now  = rpp::schedulers::clock_type::now();
        auto diff = std::chrono::milliseconds{500};

        rpp::schedulers::time_point execute_time{};
        worker->schedule(diff,
                        [&call_count, &execute_time](const auto&) -> rpp::schedulers::optional_duration
                        {
                            ++call_count;
                            execute_time = rpp::schedulers::clock_type::now();
                            return {};
                        }, obs.value());

        wait_till_finished();

        REQUIRE(call_count == 1);
        REQUIRE(execute_time - now >= diff);
    }

    SECTION("scheduler re-schedules action at provided timepoint")
    {
        std::vector<rpp::schedulers::time_point> executions{};
        std::chrono::milliseconds                diff = std::chrono::milliseconds{500};
        worker->schedule([&call_count,&executions, &diff](const auto&) -> rpp::schedulers::optional_duration
                        {
                            executions.push_back(rpp::schedulers::clock_type::now());
                            if (++call_count <= 1)
                                return diff;
                            return {};
                        }, obs.value());

        wait_till_finished();

        REQUIRE(call_count == 2);
        REQUIRE(executions[1] - executions[0] >= (diff - std::chrono::milliseconds(100)));
    }

    SECTION("scheduler with nesting scheduling should defer actual execution of tasks")
    {
        std::vector<std::string> call_stack;

        auto execution_thread = get_thread(simulate_nested_scheduling(worker.value(), obs.value(), call_stack));

        wait_till_finished();

        REQUIRE(call_stack == std::vector<std::string>{
                "Task 1 starts "s + execution_thread,
                "Task 1 ends "s + execution_thread,
                "Task 2 starts "s + execution_thread,
                "Task 2 ends "s + execution_thread,
                "Task 3 runs "s + execution_thread,
        });
    }

    SECTION("scheduler with complex scheduling should defer actual execution of tasks")
    {
        std::vector<std::string> call_stack;

        auto execution_thread = get_thread(simulate_complex_scheduling(worker.value(), obs.value(), call_stack));

        wait_till_finished();

        REQUIRE(call_stack == std::vector<std::string>{
                "Task 1 starts "s + execution_thread,
                "Task 1 ends "s + execution_thread,
                "Task 2 starts "s + execution_thread,
                "Task 2 ends "s + execution_thread,
                "Task 3 starts "s + execution_thread,
                "Task 3 ends "s + execution_thread,
                "Task 4 runs "s + execution_thread,
                "Task 2 starts "s + execution_thread,
                "Task 2 ends "s + execution_thread,
                "Task 3 starts "s + execution_thread,
                "Task 3 ends "s + execution_thread,
                "Task 4 runs "s + execution_thread,
        });
    }

    SECTION("scheduler with complex scheduling with delay should defer actual execution of tasks")
    {
        std::vector<std::string> call_stack;

        auto execution_thread = get_thread(simulate_complex_scheduling_with_delay(worker.value(), obs.value(), call_stack));

        wait_till_finished();

        REQUIRE(call_stack == std::vector<std::string>{
                "Task 1 starts "s + execution_thread,
                "Task 1 ends "s + execution_thread,
                "Task 2 starts "s + execution_thread,
                "Task 2 ends "s + execution_thread,
                "Task 3 starts "s + execution_thread,
                "Task 3 ends "s + execution_thread,
                "Task 2 starts "s + execution_thread,
                "Task 2 ends "s + execution_thread,
                "Task 3 starts "s + execution_thread,
                "Task 3 ends "s + execution_thread,
                "Task 4 runs "s + execution_thread,
                "Task 4 runs "s + execution_thread,
        });
    }

    SECTION("scheduler does nothing with disposed observer")
    {
        d->dispose();
        worker->schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
        {
            ++call_count;
            return std::chrono::nanoseconds{1};
        }, obs.value());

        wait_till_finished();

        CHECK(call_count == 0);
    }

    SECTION("scheduler does nothing with recursive disposed observer")
    {
        worker->schedule([&call_count, d, worker](const auto& obs) -> rpp::schedulers::optional_duration
        {
            d->dispose();
            worker->schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
            {
                ++call_count;
                return std::chrono::nanoseconds{1};
            },
            obs);

            return std::chrono::nanoseconds{1};
        }, obs.value());

        wait_till_finished();

        CHECK(call_count == 0);
    }

    SECTION("scheduler does not reschedule after disposing inside schedulable")
    {
        worker->schedule([&call_count, d](const auto&) -> rpp::schedulers::optional_duration
        {
            if (++call_count > 1)
                d->dispose();
            return std::chrono::nanoseconds{1};
        }, obs.value());

        wait_till_finished();

        CHECK(call_count == 2);
    }

    SECTION("scheduler does not reschedule after disposing inside recursive schedulable")
    {
        worker->schedule([&call_count, d, worker](const auto& obs) -> rpp::schedulers::optional_duration
        {
            worker->schedule([&call_count, d](const auto&) -> rpp::schedulers::optional_duration
            {
                if (++call_count > 1)
                    d->dispose();
                return std::chrono::nanoseconds{1};
            },
            obs);
            return std::nullopt;
        },
        obs.value());

        wait_till_finished();

        CHECK(call_count == 2);
    }

    SECTION("scheduler does not reschedule after disposing inside recursive schedulable")
    {
        worker->schedule([&call_count, d, worker](const auto& obs) -> rpp::schedulers::optional_duration
        {
            worker->schedule([&call_count, d](const auto&) -> rpp::schedulers::optional_duration
            {
                if (++call_count > 1)
                    d->dispose();
                return std::chrono::nanoseconds{1};
            }, obs);
            return std::nullopt;
        }, obs.value());

        wait_till_finished();

        CHECK(call_count == 2);
    }

    SECTION("scheduler does not dispatch schedulable after disposing of disposable")
    {
        worker->schedule([&call_count, d, worker](const auto& obs) -> rpp::schedulers::optional_duration
                        {
                            ++call_count;
                            worker->schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
                                            {
                                                ++call_count;
                                                return std::chrono::nanoseconds{1};
                                            },
                                            obs);
                            d->dispose();
                            return std::chrono::nanoseconds{1};
                        },
                        obs.value());

        wait_till_finished();

        CHECK(call_count == 1);
    }

    SECTION("scheduler respects to time point")
    {
        std::vector<int> executions{};
        worker->schedule([&executions, worker](const auto& obs) -> rpp::schedulers::optional_duration
                        {
                            worker->schedule(std::chrono::milliseconds{3}, [&executions](const auto&){executions.push_back(3); return rpp::schedulers::optional_duration{};}, obs);
                            worker->schedule(std::chrono::milliseconds{1}, [&executions](const auto&){executions.push_back(1); return rpp::schedulers::optional_duration{};}, obs);
                            worker->schedule(std::chrono::milliseconds{2}, [&executions](const auto&){executions.push_back(2); return rpp::schedulers::optional_duration{};}, obs);
                            return rpp::schedulers::optional_duration{};
                        },
                        obs.value());

        wait_till_finished();

        CHECK(executions == std::vector{1,2,3});
    }

    SECTION("scheduler forwards any arguments")
    {
        worker->schedule([](const auto&, int, const std::string&){ return rpp::schedulers::optional_duration{}; }, obs.value(), int{}, std::string{});
    }

    SECTION("error during schedulable")
    {
        worker->schedule([](const auto&) -> rpp::schedulers::optional_duration {throw std::runtime_error{"test"};}, obs.value());

        wait_till_finished();

        CHECK(mock_obs.get_on_error_count()  == 1);
    }

    SECTION("error during recursive schedulable")
    {
        worker->schedule([worker](const auto& obs)
        {
            worker->schedule([](const auto&) -> rpp::schedulers::optional_duration {throw std::runtime_error{"test"};}, obs);
            return rpp::schedulers::optional_duration{};
        }, obs.value());

        wait_till_finished();

        CHECK(mock_obs.get_on_error_count()  == 1);
    }
}

TEST_CASE("new_thread utilized current_thread")
{
    std::atomic_bool inner_schedule_executed{};
    auto mock = mock_observer_strategy<int>{};
    {
        auto worker = rpp::schedulers::new_thread::create_worker();
        auto obs = mock.get_observer().as_dynamic();
        obs.set_upstream(worker.get_disposable());
        worker.schedule([&inner_schedule_executed](const auto& obs)
        {
            rpp::schedulers::current_thread::create_worker().schedule([&inner_schedule_executed](const auto&)
            {
                inner_schedule_executed = true;
                return rpp::schedulers::optional_duration{};
            }, obs);

            if (inner_schedule_executed)
                throw std::logic_error{"current_thread executed inside new_thread"};
            return rpp::schedulers::optional_duration{};
        }, obs);
    }

    while (!inner_schedule_executed){};

    CHECK(inner_schedule_executed);
    CHECK(mock.get_on_error_count() == 0);
}

TEST_CASE("new_thread works till end")
{
    auto mock = mock_observer_strategy<int>{};

    rpp::source::just(1,2,3,4,5,6,7,8,9,10)
    | rpp::operators::subscribe_on(rpp::schedulers::new_thread{})
    | rpp::operators::as_blocking()
    | rpp::operators::subscribe(mock);

    CHECK(mock.get_received_values().size() == 10);
}