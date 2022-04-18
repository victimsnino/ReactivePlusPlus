//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.h"

#include <catch2/catch_test_macros.hpp>

#include <rpp/schedulers.h>
#include <rpp/operators/observe_on.h>
#include <rpp/sources.h>
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
}
