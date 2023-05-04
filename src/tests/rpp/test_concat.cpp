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
#include <rpp/sources/create.hpp>
#include <snitch/snitch_macros_test_case.hpp>

#include "mock_observer.hpp"

#include <optional>

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
    SECTION("concat of array of different observables")
    {
        auto observable = rpp::source::concat<TestType>(std::array{rpp::source::just(1, 2), rpp::source::just(1, 1)});
        observable.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1,2,1, 1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat stop if no completion")
    {
        std::optional<rpp::dynamic_observer<int>> observer{};
        auto observable = rpp::source::concat<TestType>(rpp::source::just(1, 2), rpp::source::create<int>([&](auto&& obs){ observer.emplace(std::forward<decltype(obs)>(obs).as_dynamic()); }), rpp::source::just(3));
        observable.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1,2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
        REQUIRE(observer.has_value());

        SECTION("send completion later")
        {
            observer->on_completed();

            CHECK(mock.get_received_values() == std::vector{1,2,3});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
        SECTION("send emission later")
        {
            observer->on_next(10);

            CHECK(mock.get_received_values() == std::vector{1,2,10});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 0);
        }
        SECTION("send error later")
        {
            observer->on_error({});

            CHECK(mock.get_received_values() == std::vector{1,2});
            CHECK(mock.get_on_error_count() == 1);
            CHECK(mock.get_on_completed_count() == 0);
        }
    }
}