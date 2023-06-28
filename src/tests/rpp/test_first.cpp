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

#include <rpp/operators/first.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/never.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/empty.hpp>

#include "mock_observer.hpp"
#include "copy_count_tracker.hpp"

TEST_CASE("first only emits once")
{
    auto mock = mock_observer_strategy<int>{};

    SECTION("observable of -1-| - shall see -1-|")
    {
        auto obs = rpp::source::just(1);
        obs | rpp::ops::first()
            | rpp::ops::subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1});
        CHECK(mock.get_on_completed_count() == 1);
        CHECK(mock.get_on_error_count() == 0);
    }

    SECTION("observable of -1-2-3-| - shall see -1-|")
    {
        auto obs = rpp::source::just(1, 2, 3);
        obs | rpp::ops::first()
            | rpp::ops::subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == std::vector{1});
        CHECK(mock.get_on_completed_count() == 1);
        CHECK(mock.get_on_error_count() == 0);
    }

    SECTION("observable of never - shall not see neither completed nor error event")
    {
        auto obs = rpp::source::never<int>();
        obs | rpp::ops::first()
            | rpp::ops::subscribe(mock.get_observer());
        CHECK(mock.get_received_values().empty());
        CHECK(mock.get_on_completed_count() == 0);
        CHECK(mock.get_on_error_count() == 0);
    }

    SECTION("observable of x-| - shall see error and no-completed event")
    {
        auto obs = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));
        obs | rpp::ops::first()
            | rpp::ops::subscribe(mock.get_observer());

        CHECK(mock.get_received_values().empty());
        CHECK(mock.get_on_completed_count() == 0);
        CHECK(mock.get_on_error_count() == 1);
    }

    SECTION("observable of ---| - shall see -x")
    {
        auto obs = rpp::source::empty<int>();
        obs | rpp::ops::first()
            | rpp::ops::subscribe(mock.get_observer());
        CHECK(mock.get_received_values().empty());
        CHECK(mock.get_on_completed_count() == 0);
        CHECK(mock.get_on_error_count() == 1);
    }
}

TEST_CASE("first doesn't produce extra copies")
{
    SECTION("first()")
    {
        copy_count_tracker::test_operator(rpp::ops::first(),
                                        {
                                            .send_by_copy = {.copy_count = 1, // 1 copy to final subscriber
                                                            .move_count = 0},
                                            .send_by_move = {.copy_count = 0,
                                                            .move_count = 1} // 1 move to final subscriber
                                        }, 2);
    }
}