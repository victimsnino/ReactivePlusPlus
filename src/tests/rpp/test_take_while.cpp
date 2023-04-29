//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "mock_observer.hpp"

#include <rpp/operators/take_while.hpp>
#include <rpp/sources/create.hpp>
#include <snitch/snitch.hpp>

#include <stdexcept>
#include <string>

TEST_CASE("take_while")
{
    auto mock = mock_observer_strategy<int>{};
    auto obs  = rpp::source::create<int>(
        [](const auto& sub)
        {
            int v{};
            while (!sub.is_disposed())
                sub.on_next(v++);
        });

    obs | rpp::operators::take_while([](int val) { return val <= 5; }) | rpp::operators::subscribe(mock.get_observer());

    CHECK(mock.get_received_values() == std::vector{0, 1, 2, 3, 4, 5});
}