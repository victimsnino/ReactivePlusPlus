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

#include <rpp/operators/map.hpp>
#include <rpp/sources/create.hpp>

#include "mock_observer.hpp"
#include "rpp/operators/fwd.hpp"

#include <stdexcept>
#include <string>

TEST_CASE("map modifies values and forward errors/completions")
{
    auto obs = rpp::source::create<int>([](const auto& obs)
    {
        obs.on_next(1);
        obs.on_next(2);
        obs.on_completed();
    });

    SECTION("map changes value")
    {
        mock_observer_strategy<std::string> mock{};

        obs | rpp::operators::map{[](int v){return std::string("TEST ") + std::to_string(v);}} | rpp::operators::subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector<std::string>{"TEST 1", "TEST 2"});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }


    SECTION("map with exception value")
    {
        mock_observer_strategy<int> mock{};

        obs | rpp::operators::map{[](int v) -> int { throw std::runtime_error{""}; }} | rpp::operators::subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector<int>{});
        CHECK(mock.get_on_error_count() == 1);
        CHECK(mock.get_on_completed_count() == 0);
    }
}
