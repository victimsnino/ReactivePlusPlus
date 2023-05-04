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

#include "mock_observer.hpp"

TEST_CASE("concat as source")
{
    mock_observer_strategy<int> mock{};
    SECTION("concat of solo observable")
    {
        auto observable = rpp::source::concat(rpp::source::just(1, 2));
        observable.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1,2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of multiple same observables")
    {
        auto observable = rpp::source::concat(rpp::source::just(1, 2), rpp::source::just(1, 2));
        static_assert(std::same_as<decltype(observable), rpp::base_observable<int, rpp::details::concat_strategy<rpp::details::container_with_iterator<std::array<rpp::base_observable<int, rpp::details::from_iterable_strategy<rpp::details::container_with_iterator<std::array<int, 2>>, rpp::schedulers::current_thread>>, 2>>>>>);
        observable.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1,2, 1, 2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of multiple different observables")
    {
        auto observable = rpp::source::concat(rpp::source::just(1, 2), rpp::source::just(1));
        static_assert(std::same_as<decltype(observable), rpp::base_observable<int, rpp::details::concat_strategy<rpp::details::container_with_iterator<std::array<rpp::dynamic_observable<int>, 2>>>>>);
        observable.subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1,2,1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
}