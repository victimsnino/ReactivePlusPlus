//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "rpp/disposables/fwd.hpp"
#include "rpp/schedulers/fwd.hpp"
#include <snitch/snitch.hpp>
#include <rpp/schedulers.hpp>
#include <rpp/observers/lambda_observer.hpp>

#include <chrono>
#include <optional>
#include <sstream>
#include <thread>

using namespace std::string_literals;

static std::string get_thread_id_as_string(std::thread::id id = std::this_thread::get_id())
{
    std::stringstream ss;
    ss << id;
    return ss.str();
}

static std::string simulate_nested_scheduling(const auto& worker, const auto& obs, std::vector<std::string>& out)
{
    std::thread thread([&]
    {
        worker.schedule([&](const auto&)
        {
            out.push_back("Task 1 starts "s + get_thread_id_as_string());

            worker.schedule([&](const auto&)
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
    std::thread thread([&]
    {
        worker.schedule([&](const auto&)
        {
            out.push_back("Task 1 starts "s + get_thread_id_as_string());

            worker.schedule([&](const auto&, int& counter) -> rpp::schedulers::optional_duration
            {
                out.push_back("Task 2 starts "s + get_thread_id_as_string());

                worker.schedule([&](const auto&)
                {
                    out.push_back("Task 4 runs "s + get_thread_id_as_string());
                    return rpp::schedulers::optional_duration{};
                }, obs);

                out.push_back("Task 2 ends "s + get_thread_id_as_string());
                if (counter++ < 1)
                    return rpp::schedulers::duration{};
                return std::nullopt;
            }, obs, int{});

            worker.schedule([&](const auto&, int& counter) -> rpp::schedulers::optional_duration
            {
                out.push_back("Task 3 starts "s + get_thread_id_as_string());

                out.push_back("Task 3 ends "s + get_thread_id_as_string());
                if (counter++ < 1)
                    return rpp::schedulers::duration{};
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
    std::thread thread([&]
    {
        worker.schedule([&](const auto&)
        {
            out.push_back("Task 1 starts "s + get_thread_id_as_string());

            worker.schedule([&](const auto&, int& counter) -> rpp::schedulers::optional_duration
            {
                out.push_back("Task 2 starts "s + get_thread_id_as_string());

                worker.schedule(std::chrono::milliseconds{50}, [&](const auto&)
                {
                    out.push_back("Task 4 runs "s + get_thread_id_as_string());
                    return rpp::schedulers::optional_duration{};
                }, obs);

                out.push_back("Task 2 ends "s + get_thread_id_as_string());
                if (counter++ < 1)
                    return rpp::schedulers::duration{};
                return std::nullopt;
            }, obs, int{});

            worker.schedule([&](const auto&, int& counter) -> rpp::schedulers::optional_duration
            {
                out.push_back("Task 3 starts "s + get_thread_id_as_string());

                out.push_back("Task 3 ends "s + get_thread_id_as_string());
                if (counter++ < 1)
                    return rpp::schedulers::duration{};
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
    auto d = rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()};
    auto obs = rpp::make_lambda_observer(d, [](int){ }).as_dynamic();

    auto   worker = scheduler.create_worker();

    CHECK(worker.get_disposable().is_disposed());

    size_t call_count{};

    SECTION("immediate scheduler schedules and re-schedules action immediately")
    {
        worker.schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
        {
            if (++call_count <= 1)
                return rpp::schedulers::duration{};
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
            return rpp::schedulers::duration{};
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
            return rpp::schedulers::duration{};
        }, obs);

        CHECK(call_count == 2);
    }

    SECTION("immediate scheduler forwards any arguments")
    {
        worker.schedule([](const auto&, int, const std::string&){ return rpp::schedulers::optional_duration{}; }, obs, int{}, std::string{});
    }
}

TEST_CASE("current_thread scheduler")
{
    auto d = rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()};
    auto obs = rpp::make_lambda_observer(d, [](int){ }).as_dynamic();

    auto worker = rpp::schedulers::current_thread::create_worker();
    CHECK(worker.get_disposable().is_disposed());
    size_t call_count{};

    SECTION("current_thread scheduler schedules and re-schedules action immediately")
    {
        worker.schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
        {
            if (++call_count <= 1)
                return rpp::schedulers::duration{};
            return std::nullopt;
        }, obs);

        CHECK(call_count == 2);
    }

    SECTION("current_thread scheduler recursive scheduling")
    {
        worker.schedule([&call_count, &worker, &obs](const auto&) -> rpp::schedulers::optional_duration
        {
            worker.schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
            {
                if (++call_count <= 1)
                    return std::chrono::nanoseconds{1};
                return std::nullopt;
            }, obs);
            return std::nullopt;
        }, obs);

        CHECK(call_count == 2);
    }

    SECTION("current_thread scheduler recursive scheduling with original")
    {
        worker.schedule(
            [&call_count, &worker, &obs](const auto&) -> rpp::schedulers::optional_duration
            {
                worker.schedule(
                    [&call_count](const auto&) -> rpp::schedulers::optional_duration
                    {
                        if (++call_count <= 1)
                            return std::chrono::nanoseconds{1};
                        return std::nullopt;
                    },
                    obs);
                if (call_count == 0)
                    return std::chrono::nanoseconds{1};
                return std::nullopt;
            },
            obs);

        CHECK(call_count == 3);
    }

    SECTION("current_thread scheduler schedules action with delay")
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

    SECTION("current_thread scheduler re-schedules action at provided timepoint")
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

    SECTION("current_thread scheduler with nesting scheduling should defer actual execution of tasks")
    {
        std::vector<std::string> call_stack;

            auto execution_thread = simulate_nested_scheduling(worker, obs, call_stack);

            REQUIRE(call_stack == std::vector<std::string>{
                    "Task 1 starts "s + execution_thread,
                    "Task 1 ends "s + execution_thread,
                    "Task 2 starts "s + execution_thread,
                    "Task 2 ends "s + execution_thread,
                    "Task 3 runs "s + execution_thread,
            });
    }

    SECTION("current_thread scheduler with complex scheduling should defer actual execution of tasks")
    {
        std::vector<std::string> call_stack;

            auto execution_thread = simulate_complex_scheduling(worker, obs, call_stack);

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

    SECTION("current_thread scheduler with complex scheduling with delay should defer actual execution of tasks")
    {
        std::vector<std::string> call_stack;

            auto execution_thread = simulate_complex_scheduling_with_delay(worker, obs, call_stack);

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

    SECTION("current_thread scheduler does nothing with disposed observer")
    {
        d.dispose();
        worker.schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
        {
            ++call_count;
            return rpp::schedulers::duration{};
        }, obs);

        CHECK(call_count == 0);
    }

    SECTION("current_thread scheduler does not reschedule after disposing inside schedulable")
    {
        worker.schedule([&call_count, &d](const auto&) -> rpp::schedulers::optional_duration
        {
            if (++call_count > 1)
                d.dispose();
            return rpp::schedulers::duration{};
        }, obs);

        CHECK(call_count == 2);
    }

    SECTION("current_thread scheduler does not reschedule after disposing inside recursive schedulable")
    {
        worker.schedule([&call_count, &d, &worker](const auto& obs) -> rpp::schedulers::optional_duration
        {
            worker.schedule([&call_count, &d](const auto&) -> rpp::schedulers::optional_duration
            {
                if (++call_count > 1)
                    d.dispose();
                return rpp::schedulers::duration{};
            },
            obs);
            return std::nullopt;
        },
        obs);

        CHECK(call_count == 2);
    }

    SECTION("current_thread scheduler does not reschedule after disposing inside recursive schedulable")
    {
        worker.schedule([&call_count, &d, &worker](const auto& obs) -> rpp::schedulers::optional_duration
        {
            worker.schedule([&call_count, &d](const auto&) -> rpp::schedulers::optional_duration
            {
                if (++call_count > 1)
                    d.dispose();
                return rpp::schedulers::duration{};
            }, obs);
            return std::nullopt;
        }, obs);

        CHECK(call_count == 2);
    }

    SECTION("current_thread scheduler does not dispatch schedulable after disposing of disposable")
    {
        worker.schedule([&call_count, &d, &worker](const auto& obs) -> rpp::schedulers::optional_duration
                        {
                            ++call_count;
                            worker.schedule([&call_count](const auto&) -> rpp::schedulers::optional_duration
                                            {
                                                ++call_count;
                                                return rpp::schedulers::duration{};
                                            },
                                            obs);
                            d.dispose();
                            return rpp::schedulers::duration{};
                        },
                        obs);

        CHECK(call_count == 1);
    }

    SECTION("current_thread scheduler respects to time point")
    {
        std::vector<int> executions{};
        worker.schedule([&executions, &worker](const auto& obs) -> rpp::schedulers::optional_duration
                        {
                            worker.schedule(std::chrono::milliseconds{3}, [&executions](const auto&){executions.push_back(3); return rpp::schedulers::optional_duration{};}, obs);
                            worker.schedule(std::chrono::milliseconds{1}, [&executions](const auto&){executions.push_back(1); return rpp::schedulers::optional_duration{};}, obs);
                            worker.schedule(std::chrono::milliseconds{2}, [&executions](const auto&){executions.push_back(2); return rpp::schedulers::optional_duration{};}, obs);
                            return rpp::schedulers::optional_duration{};
                        },
                        obs);

        CHECK(executions == std::vector{1,2,3});
    }

    SECTION("current_thread scheduler forwards any arguments")
    {
        worker.schedule([](const auto&, int, const std::string&){ return rpp::schedulers::optional_duration{}; }, obs, int{}, std::string{});
    }
}

