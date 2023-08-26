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

#include <rpp/sources/defer.hpp>
#include <rpp/sources/from.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/never.hpp>

#include "mock_observer.hpp"

TEST_CASE("defer on different sources")
{
    auto mock = mock_observer_strategy<int>{};
    
    SECTION("just")
    {
        auto obs = rpp::source::defer([] { return rpp::source::just(1); });
        obs.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{ 1 });
        CHECK(mock.get_total_on_next_count() == 1);
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("error")
    {
        auto obs = rpp::source::defer([] { return rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{"error"})); });
        obs.subscribe(mock.get_observer());

        CHECK(mock.get_total_on_next_count() == 0);
        CHECK(mock.get_on_error_count() == 1);
        CHECK(mock.get_on_completed_count() == 0);
    }
    SECTION("empty")
    {
        auto obs = rpp::source::defer([] { return rpp::source::empty<int>(); });
        obs.subscribe(mock.get_observer());

        CHECK(mock.get_total_on_next_count() == 0);
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("never")
    {
        auto obs = rpp::source::defer([] { return rpp::source::never<int>(); });
        obs.subscribe(mock.get_observer());

        CHECK(mock.get_total_on_next_count() == 0);
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
    }
}