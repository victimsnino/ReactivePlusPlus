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


#include <catch2/catch_test_macros..hpp>
#include <rpp/observables.hpp>
#include <rpp/operators/map.hpp>

using namespace std::string_literals;

SCENARIO("Map changes values", "[map]")
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
}

SCENARIO("Map doesn't produce extra copies", "[map][track_copy]")
{
    GIVEN("Observable and observer")
    {
        auto observer   = rpp::specific_observer([](const bool&){});
        auto tracker    = copy_count_tracker{};


        WHEN("subscribe on observable with map and send by copy")
        {
            auto observable = rpp::observable::create<copy_count_tracker>([&](const auto& sub)
            {
                sub.on_next(tracker);
            });

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
            auto observable = rpp::observable::create<copy_count_tracker>([&](const auto& sub)
            {
                sub.on_next(std::move(tracker));
            });

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
    }
}
