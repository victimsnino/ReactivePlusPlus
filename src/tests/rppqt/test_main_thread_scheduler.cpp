//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"

#include <snitch/snitch.hpp>

#include <rppqt/schedulers/main_thread.hpp>
#include <rpp/observers/dynamic_observer.hpp>

#include <future>

#include <QApplication>

TEST_CASE("main_thread_scheduler schedules actions to main thread")
{
    auto observer = mock_observer_strategy<int>{}.get_observer().as_dynamic();

    int argc{};
    QCoreApplication application{argc, nullptr};
    QTimer::singleShot(10, &application, [&]{application.exit();});


    SECTION("submitting action to main scheduler from another thread")
    {
        std::promise<std::thread::id> execution_thread{};
        std::thread{[&]
        {
            rppqt::schedulers::main_thread_scheduler::create_worker().schedule([&](const auto&)->rpp::schedulers::optional_delay_from_now
            {
                execution_thread.set_value(std::this_thread::get_id());
                return {};
            }, observer);
        }}.join();

        application.exec();
        SECTION("thread of exectuion of schedulable should be same as thread of application")
        {
            auto future = execution_thread.get_future();
            REQUIRE(future.wait_for(std::chrono::seconds{1}) == std::future_status::ready);
            CHECK(future.get() == std::this_thread::get_id());
        }
    }

    SECTION("recursive scheduling to main thread")
    {
        std::string execution{};
        std::thread{[&]
        {
            rppqt::schedulers::main_thread_scheduler::create_worker().schedule([&](const auto&)->rpp::schedulers::optional_delay_from_now
            {
                rppqt::schedulers::main_thread_scheduler::create_worker().schedule([&](const auto&)->rpp::schedulers::optional_delay_from_now
                {
                    execution += "inner ";
                    return {};
                }, observer);
                
                const bool first_run = execution.empty();
                execution += "outer ";
                return first_run ? rpp::schedulers::optional_delay_from_now{std::chrono::nanoseconds{}} : std::nullopt;
            }, observer);
        }}.join();

        application.exec();
        CHECK(execution == "outer inner outer inner ");
    }
}
