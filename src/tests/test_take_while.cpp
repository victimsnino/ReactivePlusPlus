//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "mock_observer.hpp"
#include "rpp/sources/create.hpp"

#include <rpp/operators/take_while.hpp>
#include <catch2/catch_test_macros..hpp>

SCENARIO("take_while filters values")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable")
    {
        auto obs = rpp::observable::create<int>([](const auto& sub)
        {
            int v{};
            while (sub.is_subscribed())
                sub.on_next(v++);
        });
        WHEN("subscribe on it with take_while")
        {
            obs.take_while([](int val)
            {
                return val <= 5;
            }).subscribe(mock);

            THEN("only items before false obtained")
            {
                CHECK(mock.get_received_values() == std::vector{ 0, 1, 2, 3, 4, 5 });
            }
        }
    }
}
