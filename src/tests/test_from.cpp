//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "copy_count_tracker.hpp"
#include "mock_observer.hpp"

#include <rpp/sources/from.hpp>
#include <rpp/schedulers/new_thread_scheduler.hpp>
#include <catch2/catch_test_macros.hpp>

SCENARIO("from iterable")
{
    auto mock = mock_observer<int>();
    GIVEN("observable from iterable")
    {
        auto vals = std::vector{1, 2, 3, 4, 5, 6};
        auto obs  = rpp::source::from_iterable(vals);
        WHEN("subscribe on it")
        {
            obs.subscribe(mock);
            THEN("observer obtains values in the same order")
            {
                CHECK(mock.get_received_values() == vals);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    GIVEN("observable from iterable with scheduler")
    {
        auto vals = std::vector{ 1, 2, 3, 4, 5, 6 };
        auto obs = rpp::source::from_iterable(vals, rpp::schedulers::new_thread{}).as_blocking();
        WHEN("subscribe on it")
        {
            obs.subscribe(mock);
            THEN("observer obtains values in the same order")
            {
                CHECK(mock.get_received_values() == vals);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

SCENARIO("from iterable", "[track_copy]")
{
    copy_count_tracker tracker{};
    auto               vals         = std::array{tracker};
    auto               initial_copy = tracker.get_copy_count();
    auto               initial_move = tracker.get_move_count();

    GIVEN("observable from copied iterable")
    {
        auto obs = rpp::source::from_iterable(vals);
        WHEN("subscribe on it")
        {
            obs.subscribe();
            THEN("no extra copies")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 1); // 1 copy to wrapped container
                CHECK(tracker.get_move_count() - initial_move <= 2); // 1 move to lambda, 1 move lambda to observable
            }
        }
    }
    GIVEN("observable from moved iterable")
    {
        auto obs = rpp::source::from_iterable(std::move(vals));
        WHEN("subscribe on it")
        {
            obs.subscribe();
            THEN("no extra copies")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 0); 
                CHECK(tracker.get_move_count() - initial_move <= 3); // 1 move to wrapped container + 1 move to lambda, 1 move lambda to observable
            }
        }
    }

    GIVEN("observable from copied iterable with shared memory model")
    {
        auto obs = rpp::source::from_iterable<rpp::memory_model::use_shared>(vals);
        WHEN("subscribe on it")
        {
            obs.subscribe();
            THEN("no extra copies")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 1); // 1 copy to shared_ptr
                CHECK(tracker.get_move_count() - initial_move == 0);
            }
        }
    }
    GIVEN("observable from moved iterable")
    {
        auto obs = rpp::source::from_iterable<rpp::memory_model::use_shared>(std::move(vals));
        WHEN("subscribe on it")
        {
            obs.subscribe();
            THEN("no extra copies")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 0);
                CHECK(tracker.get_move_count() - initial_move == 1); // 1 copy to shared_ptr
            }
        }
    }
}

SCENARIO("from callable")
{
    GIVEN("observable from callable")
    {
        size_t count_of_calls{};
        auto callable = [&count_of_calls]() -> int {return static_cast<int>(++count_of_calls); };
        auto observable = rpp::source::from_callable(callable);
        WHEN("subscribe on this observable")
        {
            auto mock = mock_observer<int>{};
            observable.subscribe(mock);
            THEN("callable called only once and observable returns value of this function")
            {
                CHECK(mock.get_received_values() == std::vector{ 1 });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(count_of_calls == 1);
            }
        }
    }
}