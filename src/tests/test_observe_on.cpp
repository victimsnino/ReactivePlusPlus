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
#include <rpp/operators/observe_on.hpp>
#include <rpp/sources.hpp>
#include <set>


SCENARIO("observe_on transfers emssions to scheduler", "[operators][observe_on]")
{
    auto mock = mock_observer<std::string>{};
    auto scheduler = test_scheduler{};

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
                CHECK(scheduler.get_schedulings() == std::vector{ s_current_time, s_current_time, s_current_time });//2 items + on_completed 
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
                CHECK(scheduler.get_schedulings() == std::vector{ s_current_time });

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
                CHECK(scheduler.get_schedulings() == std::vector{ s_current_time });

            }
        }
    }
}

SCENARIO("observe_on with immediate doesn't produce a lot of copies", "[operators][observe_on][track_copy]")
{
    GIVEN("observable with value by copy")
    {
        auto tracker = copy_count_tracker{};
        WHEN("subscribe on it via scheduler")
        {
            tracker.get_observable().observe_on(rpp::schedulers::immediate{}).subscribe();
            THEN("only 1 extra copy")
            {
                CHECK(tracker.get_copy_count() == 1);
                CHECK(tracker.get_move_count() == 0);
            }
        }
    }
    GIVEN("observable with value by move")
    {
        auto tracker = copy_count_tracker{};
        WHEN("subscribe on it via scheduler")
        {
            tracker.get_observable_for_move().observe_on(rpp::schedulers::immediate{}).subscribe();
            THEN("only 1 extra move")
            {
                CHECK(tracker.get_copy_count() == 0);
                CHECK(tracker.get_move_count() == 1);
            }
        }
    }
}
