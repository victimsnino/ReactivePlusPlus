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

#include <rpp/operators/filter.hpp>
#include <rpp/sources/create.hpp>

#include "mock_observer.hpp"

#include <stdexcept>
#include <string>

TEST_CASE("filter")
{
    mock_observer_strategy<int> mock{};

    auto obs = rpp::source::create<int>([](const auto& obs)
    {
        obs.on_next(1);
        obs.on_next(2);
        obs.on_next(3);
        obs.on_next(4);
        obs.on_completed();
    });

    SECTION("filter emits only satisfying values")
    {
        obs | rpp::operators::filter([](auto v){return v % 2 == 0;}) | rpp::operators::subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{2, 4});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }


    SECTION("filter with exception value")
    {
        mock_observer_strategy<int> mock{};

        obs | rpp::operators::filter([](int) -> bool { throw std::runtime_error{""}; }) | rpp::operators::subscribe(mock.get_observer()); // NOLINT

        CHECK(mock.get_received_values() == std::vector<int>{});
        CHECK(mock.get_on_error_count() == 1);
        CHECK(mock.get_on_completed_count() == 0);
    }
}
