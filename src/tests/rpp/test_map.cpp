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
#include <rpp/sources/just.hpp>

#include "mock_observer.hpp"
#include "copy_count_tracker.hpp"

#include <stdexcept>
#include <string>

TEMPLATE_TEST_CASE("map modifies values and forward errors/completions", "", rpp::memory_model::use_stack, rpp::memory_model::use_shared)
{
    auto obs = rpp::source::just<TestType>(1,2);

    SECTION("map changes value")
    {
        mock_observer_strategy<std::string> mock{};

        obs | rpp::operators::map([](auto v){return std::string("TEST ") + std::to_string(v);}) | rpp::operators::subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector<std::string>{"TEST 1", "TEST 2"});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }


    SECTION("map with exception value")
    {
        mock_observer_strategy<int> mock{};

        auto map = rpp::operators::map([](int) -> int { throw std::runtime_error{""}; });

        obs | map | rpp::operators::subscribe(mock.get_observer()); // NOLINT

        CHECK(mock.get_received_values() == std::vector<int>{});
        CHECK(mock.get_on_error_count() == 1);
        CHECK(mock.get_on_completed_count() == 0);
    }
}



TEST_CASE("map doesn't produce extra copies")
{
    SECTION("map([](auto&& v){return std::forward(v);})")
    {
        copy_count_tracker::test_operator(rpp::ops::map([](auto&& v){return std::forward<decltype(v)>(v);}),
                                        {
                                            .send_by_copy = {.copy_count = 1, // 1 copy on return from map
                                                            .move_count = 1}, // 1 move to final subscriber
                                            .send_by_move = {.copy_count = 0,
                                                            .move_count = 2} // 1 move on return from map + 1 move to final subscriber
                                        });
    }
}