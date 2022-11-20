//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <catch2/catch_test_macros.hpp>

#include <rppqt/schedulers/main_thread_scheduler.hpp>

#include <future>
#include <QApplication>

SCENARIO("main_thread_scheduler schedules actions to main thread", "[schedulers]")
{
    GIVEN("qapplication and scheduler")
    {
        int argc{};
        QApplication application{argc, nullptr};
        WHEN("submitting action to main scheduler from another thread")
        {
            std::promise<std::thread::id> execution_thread{};
            std::thread{[&]
            {
                rppqt::schedulers::main_thread_scheduler{}.create_worker().schedule([&]()->rpp::schedulers::optional_duration
                {
                    execution_thread.set_value(std::this_thread::get_id());
                    application.exit();
                    return {};
                });
            }}.join();
            application.exec();
            THEN("thread of exectuion of schedulable should be same as thread of application")
            {
                auto future = execution_thread.get_future();
                REQUIRE(future.wait_for(std::chrono::seconds{3}) == std::future_status::ready);
                CHECK(future.get() == std::this_thread::get_id());
            }
        }
    }
}