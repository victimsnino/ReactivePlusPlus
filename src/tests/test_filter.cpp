//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <catch2/catch_test_macros.hpppp>
#include <rpp/operators/filter.hpp>
#include <rpp/sources/create.hpp>

#include "mock_observer.hpp"

SCENARIO("Filter provides only satisfied items")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable")
    {
        auto observable = rpp::source::create<int>([](const auto& sub)
        {
            for (int i = 1; i < 5; ++i)
                sub.on_next(i);
        });

        WHEN("subscribe on filtered observable")
        {
            observable.filter([](int v)
            {
                return v % 2 == 0;
            }).subscribe(mock);

            THEN("obtained only satisfied items")
            {
                CHECK(mock.get_received_values() == std::vector<int>{2, 4});
            }
        }
    }
}
