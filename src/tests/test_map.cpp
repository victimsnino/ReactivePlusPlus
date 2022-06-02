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

#include <rpp/sources/create.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>


#include <catch2/catch_test_macros.hpp>
#include <rpp/observables.hpp>
#include <rpp/operators/map.hpp>

using namespace std::string_literals;

SCENARIO("Map changes values", "[operators][map]")
{
    GIVEN("Observable and observer")
    {
        auto observer = mock_observer<int>();
        auto observable = rpp::observable::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_next(2);
            sub.on_next(3);
        });

        WHEN("subscribe on observable with map")
        {
            auto new_obs = observable.map([](const int& v) {return v*10;});
            new_obs.subscribe(observer);
            THEN("observer obtains modified values")
            {
                CHECK(observer.get_received_values() == std::vector{10, 20, 30});
            }
        }

        WHEN("subscribe on observable with map with exception")
        {
            volatile bool none{};

            auto new_obs = observable.map([&](const int& ) -> int
            {
                if (!none)
                    throw std::runtime_error{""};
                return 1;
            });
            new_obs.subscribe(observer);
            THEN("observer obtains modified values")
            {
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 1);
                CHECK(observer.get_on_completed_count() == 0);
            }
        }
    }

    GIVEN("Observable and observer of different types")
    {
        auto observer = mock_observer<std::string>();
        auto observable = rpp::observable::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_next(2);
            sub.on_next(3);
        });

        WHEN("subscribe on observable with map")
        {
            auto new_obs = observable.map([](const auto& v) { return std::to_string(v * 10); });
            new_obs.subscribe(observer);
            THEN("observer obtains modified values")
            {
                CHECK(observer.get_received_values() == std::vector{"10"s, "20"s, "30"s});
            }
        }
    }

    GIVEN("Observable with error and observer")
    {
        auto observer = mock_observer<int>();
        auto observable = rpp::observable::error<int>(std::make_exception_ptr(std::runtime_error{""}));

        WHEN("subscribe on observable with map")
        {
            auto new_obs = observable.map([](const int& v) {return v * 10; });
            new_obs.subscribe(observer);
            THEN("observer obtains error")
            {
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 1);
                CHECK(observer.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("Map doesn't produce extra copies", "[operators][map][track_copy]")
{
    GIVEN("Observable and observer")
    {
        auto observer   = rpp::specific_observer([](const bool&){});
        auto tracker    = copy_count_tracker{};

        WHEN("subscribe on observable with map and send by copy")
        {
            auto observable = tracker.get_observable();

            auto new_obs = observable.map([&](copy_count_tracker)
            {
                return true;
            });
            new_obs.subscribe(observer);

            THEN("only one copy to pass inside map")
            {
                CHECK(tracker.get_copy_count() == 1);
                CHECK(tracker.get_move_count() == 0);
            }
        }

        WHEN("subscribe on observable with map and send by move")
        {
            auto observable = tracker.get_observable_for_move();
            auto new_obs = observable.map([&](copy_count_tracker)
            {
                return true;
            });
            new_obs.subscribe(observer);

            THEN("only one move to pass inside map")
            {
                CHECK(tracker.get_copy_count() == 0);
                CHECK(tracker.get_move_count() == 1);
            }
        }

        WHEN("subscribe on observable with return from map by copy")
        {
            rpp::source::just(1).map([&](const auto&) {return tracker; }).subscribe([](copy_count_tracker tracker){});
            THEN("only one copy from map and move to lambda")
            {
                CHECK(tracker.get_copy_count() == 1);
                CHECK(tracker.get_move_count() == 1);
            }
        }
        WHEN("subscribe on observable with return from map by move")
        {
            rpp::source::just(1).map([](const auto&)
                                 {
                                     return copy_count_tracker{};
                                 }).
                                 subscribe([](copy_count_tracker tracker)
                                 {
                                     THEN("only one move to lambda")
                                     {
                                         CHECK(tracker.get_copy_count() == 0);
                                         CHECK(tracker.get_move_count() == 1);
                                     }
                                 });
        }
    }
}
