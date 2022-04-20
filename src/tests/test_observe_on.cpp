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

#include <catch2/catch_test_macros..hpp>

#include <rpp/schedulers.hpp>
#include <rpp/operators/observe_on.hpp>
#include <rpp/sources.hpp>
#include <set>

SCENARIO("observe_on transfers emssions to scheduler")
{
    auto mock = mock_observer<std::string>{};

    GIVEN("observable with item")
    {
        auto vals = std::vector<std::string>{"2", "3"};
        auto obs  = rpp::source::from(vals);

        WHEN("subscribe on observable via observe_on with immediate scheduler")
        {
            auto res = obs.observe_on(rpp::schedulers::immediate{});
            THEN("obtain values in the same order")
            {
                res.subscribe(mock);

                CHECK(mock.get_received_values() == vals);
                CHECK(mock.get_on_completed_count() == 1);

            }
            THEN("obtain values in the same thread")
            {
                std::set<std::thread::id>     threads{};
                res.subscribe([&](const auto& v)
                {
                    threads.insert(std::this_thread::get_id());
                });

                CHECK(threads == std::set{std::this_thread::get_id()});
            }
        }

        WHEN("subscribe on observable via observe_on with new_thread scheduler")
        {
            auto res = obs.observe_on(rpp::schedulers::new_thread{});
            THEN("obtain values in the same order")
            {
                res.as_blocking().subscribe(mock);

                CHECK(mock.get_received_values() == vals);
                CHECK(mock.get_on_completed_count() == 1);

            }
            THEN("obtain values in the same thread")
            {
                std::set<std::thread::id>                   threads{};
                res.as_blocking().subscribe([&](const auto& v)
                {
                    threads.insert(std::this_thread::get_id());
                });

                CHECK(threads != std::set{ std::this_thread::get_id() });
            }
        }
    }

    GIVEN("observable with error")
    {
        auto obs = rpp::source::error<std::string>(std::make_exception_ptr(std::runtime_error{""}));

        WHEN("subscribe on observable via observe_on with immediate scheduler")
        {
            auto res = obs.observe_on(rpp::schedulers::immediate{});
            THEN("obtain error")
            {
                res.subscribe(mock);

                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }


        WHEN("subscribe on observable via observe_on with immediate scheduler")
        {
            auto res = obs.observe_on(rpp::schedulers::new_thread{});
            THEN("obtain error")
            {
                res.as_blocking().subscribe(mock);

                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("observe_on doesn't produce a lot of copies", "[track_copy]")
{
    GIVEN("observable with value by copy")
    {
        auto tracker = copy_count_tracker{};
        auto obs     = rpp::source::create<copy_count_tracker>([&](const auto& sub)
        {
            sub.on_next(tracker);
        });
        WHEN("subscribe on it via scheduler")
        {
            obs.observe_on(rpp::schedulers::immediate{}).subscribe();
            THEN("only 1 extra copy and 1 move")
            {
                CHECK(tracker.get_copy_count() == 1);
                CHECK(tracker.get_move_count() == 1);
            }
        }
    }
    GIVEN("observable with value by move")
    {
        auto tracker = copy_count_tracker{};
        auto obs     = rpp::source::create<copy_count_tracker>([&](const auto& sub)
        {
            sub.on_next(std::move(tracker));
        });
        WHEN("subscribe on it via scheduler")
        {
            obs.observe_on(rpp::schedulers::immediate{}).subscribe();
            THEN("only 2 extra moves")
            {
                CHECK(tracker.get_copy_count() == 0);
                CHECK(tracker.get_move_count() == 2);
            }
        }
    }
}
