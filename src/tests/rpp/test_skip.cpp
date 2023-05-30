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

#include <rpp/operators/skip.hpp>
#include <rpp/sources/create.hpp>

#include "mock_observer.hpp"

TEST_CASE("skip ignores first `count` of items")
{
    auto mock = mock_observer_strategy<int>{};
    auto obs = rpp::source::create<int>([](const auto& sub)
    {
        for (int i = 0; i < 10; ++i)
            sub.on_next(i);
        sub.on_completed();
    });

    SECTION("subscribe to observable of 10 items via skip(5) skips first 5 items")
    {
        constexpr size_t count = 5;
        auto             new_obs = obs| rpp::operators::skip(count);
        new_obs.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{ 5,6,7,8,9});
        CHECK(mock.get_on_completed_count() == 1);

        SECTION("second subscription sees same")
        {
            auto mock_2 = mock_observer_strategy<int>{};
            new_obs.subscribe(mock_2.get_observer());

            CHECK(mock_2.get_received_values() == std::vector{ 5,6,7,8,9 });
            CHECK(mock.get_on_completed_count() == 1);
        }
    }
    SECTION("subscribe to observable of 10 via skip(0) emits all values")
    {
        constexpr size_t count = 0;
        auto             new_obs = obs | rpp::ops::skip(count);
        new_obs.subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == std::vector{ 0,1,2,3,4,5,6,7,8,9 });
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("subscribe to observable of 10 via skip(1000) emits nothing but completes")
    {
        constexpr size_t count = 1000;
        auto             new_obs = obs | rpp::ops::skip(count);
        new_obs.subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == std::vector<int>{ });
        CHECK(mock.get_on_completed_count() == 1);
    }
}