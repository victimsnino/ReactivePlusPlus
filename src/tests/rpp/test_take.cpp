//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <snitch/snitch.hpp>

#include <rpp/operators/take.hpp>
#include <rpp/sources/create.hpp>

#include "mock_observer.hpp"

TEST_CASE("take operator can be applied to observable")
{
    constexpr size_t create_count = 3;
    auto obs = rpp::source::create<int>([](const auto& obs)
    {
        for (size_t i = 0; i < create_count; ++i)
        {
            obs.on_next(static_cast<int>(i));
        }
        CHECK(obs.is_disposed());
    });

    mock_observer_strategy<int> mock{};


    SECTION("subscribe via take(3)")
    {
        (obs | rpp::operators::take{create_count}).subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{0, 1, 2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
 }