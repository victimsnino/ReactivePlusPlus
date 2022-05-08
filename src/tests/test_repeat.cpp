//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"
#include "rpp/sources.hpp"

#include <rpp/operators/repeat.hpp>
#include <rpp/operators/take.hpp>
#include <catch2/catch_test_macros.hpp>

SCENARIO("repeat resubscribes")
{
    auto observer = mock_observer<int>();
    GIVEN("observable with value")
    {
        size_t subscribe_count = 0;
        auto   observable      = rpp::source::create<int>([&subscribe_count](const auto& sub)
        {
            ++subscribe_count;
            sub.on_next(1);
            sub.on_completed();
        });
        WHEN("subscribe on it via repeat(0)")
        {
            observable.repeat(0).subscribe(observer);
            THEN("only on_completed sent")
            {
                CHECK(subscribe_count == 0);
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via repeat(1)")
        {
            observable.repeat(1).subscribe(observer);
            THEN("sent value once")
            {
                CHECK(subscribe_count == 1);
                CHECK(observer.get_total_on_next_count() == 1);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via repeat(10)")
        {
            observable.repeat(10).subscribe(observer);
            THEN("sent value 10 times")
            {
                CHECK(subscribe_count == 10);
                CHECK(observer.get_total_on_next_count() == 10);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via repeat()")
        {
            observable.repeat().take(10).subscribe(observer);
            THEN("sent value infinitely")
            {
                CHECK(subscribe_count == 10);
                CHECK(observer.get_total_on_next_count() == 10);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
    }
    GIVEN("observable with on_error")
    {
        size_t subscribe_count = 0;
        auto   observable      = rpp::source::create<int>([&subscribe_count](const auto& sub)
        {
            ++subscribe_count;
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
        });
        WHEN("subscribe on it via repeat(10)")
        {
            observable.repeat(10).subscribe(observer);
            THEN("only on_error once")
            {
                CHECK(subscribe_count == 1);
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 1);
                CHECK(observer.get_on_completed_count() == 0);
            }
        }
    }
}
