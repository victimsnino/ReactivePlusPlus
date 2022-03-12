// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "copy_count_tracker.h"
#include "mock_observer.h"

#include <catch2/catch_test_macros.hpp>
#include <rpp/observable.h>
#include <rpp/operators/map.h>

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
            auto new_obs = observable | rpp::operators::map([](const int& v) {return v*10;});
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
            auto new_obs = observable | rpp::operators::map([](const auto& v) { return std::to_string(v * 10); });
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

            auto new_obs = observable | rpp::operators::map([&](copy_count_tracker)
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

            auto new_obs = observable | rpp::operators::map([&](copy_count_tracker)
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
