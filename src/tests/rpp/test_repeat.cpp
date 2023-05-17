//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <snitch/snitch.hpp>

#include <rpp/operators/repeat.hpp>
#include <rpp/operators/take.hpp>
#include <rpp/sources/create.hpp>

#include "mock_observer.hpp"


TEST_CASE("repeat resubscribes")
{
    auto observer = mock_observer_strategy<int>();
    SECTION("observable with value")
    {
        size_t subscribe_count = 0;
        auto   observable      = rpp::source::create<int>([&subscribe_count](const auto& sub)
        {
            ++subscribe_count;
            sub.on_next(1);
            sub.on_completed();
        });

        SECTION("subscribe on it via repeat(0)")
        {
            observable | rpp::operators::repeat(0) | rpp::operators::subscribe(observer.get_observer());
            SECTION("only on_completed sent")
            {
                CHECK(subscribe_count == 0);
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
        SECTION("subscribe on it via repeat(1)")
        {
            observable| rpp::operators::repeat(1) | rpp::operators::subscribe(observer.get_observer());
            SECTION("sent value once")
            {
                CHECK(subscribe_count == 1);
                CHECK(observer.get_total_on_next_count() == 1);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
        SECTION("subscribe on it via repeat(10)")
        {
            observable| rpp::operators::repeat(10) | rpp::operators::subscribe(observer.get_observer());
            SECTION("sent value 10 times")
            {
                CHECK(subscribe_count == 10);
                CHECK(observer.get_total_on_next_count() == 10);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
        SECTION("subscribe on it via repeat()")
        {
            observable | rpp::operators::repeat() | rpp::operators::take(10) | rpp::operators::subscribe(observer.get_observer());
            SECTION("sent value infinitely")
            {
                CHECK(subscribe_count == 10);
                CHECK(observer.get_total_on_next_count() == 10);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
    }
    SECTION("observable with on_error")
    {
        size_t subscribe_count = 0;
        auto   observable      = rpp::source::create<int>([&subscribe_count](const auto& sub)
        {
            ++subscribe_count;
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
        });
        SECTION("subscribe on it via repeat(10)")
        {
            observable| rpp::operators::repeat(10) | rpp::operators::subscribe(observer.get_observer());
            SECTION("only on_error once")
            {
                CHECK(subscribe_count == 1);
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 1);
                CHECK(observer.get_on_completed_count() == 0);
            }
        }
    }
    SECTION("observable with on_completed")
    {
        size_t subscribe_count = 0;
        auto   observable = rpp::source::create<int>([&subscribe_count](const auto& sub)
            {
                ++subscribe_count;
                sub.on_completed();
            });
        SECTION("subscribe on it via repeat(10)")
        {
            observable| rpp::operators::repeat(10) | rpp::operators::subscribe(observer.get_observer());
            SECTION("on_ompleted once")
            {
                CHECK(subscribe_count == 10);
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
    }
}