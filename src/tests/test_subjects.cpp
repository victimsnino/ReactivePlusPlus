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
#include <rpp/subjects/publish_subject.h>

SCENARIO("publish subject multicasts values")
{
    auto mock_1 = mock_observer<int>{};
    auto mock_2 = mock_observer<int>{};
    GIVEN("publish subject")
    {
        auto sub = rpp::subjects::publish_subject<int>{};
        WHEN("subsribe multiple observers and emit values")
        {
            sub.get_observable().subscribe(mock_1);
            sub.get_observable().subscribe(mock_2);

            sub.get_subscriber().on_next(1);
            THEN("observers obtain value")
            {
                CHECK(mock_1.get_received_values() == mock_2.get_received_values());
                CHECK(mock_1.get_received_values() ==std::vector{1});
            }
        }
    }
}