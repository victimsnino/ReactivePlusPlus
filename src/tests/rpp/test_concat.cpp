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
#include <rpp/sources/just.hpp>
#include <rpp/sources/concat.hpp>
#include <snitch/snitch_macros_test_case.hpp>

#include "mock_observer.hpp"
#include "rpp/memory_model.hpp"

TEMPLATE_TEST_CASE("concat as source", "", rpp::memory_model::use_stack, rpp::memory_model::use_shared)
{
    mock_observer_strategy<int> mock{};
    SECTION("concat of solo observable")
    {
        auto observable = rpp::source::concat<TestType>(rpp::source::just(1, 2));
        observable.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1,2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of multiple same observables")
    {
        auto observable = rpp::source::concat<TestType>(rpp::source::just(1, 2), rpp::source::just(1, 2));
        observable.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1,2, 1, 2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of multiple different observables")
    {
        auto observable = rpp::source::concat<TestType>(rpp::source::just(1, 2), rpp::source::just(1));
        observable.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1,2,1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
}