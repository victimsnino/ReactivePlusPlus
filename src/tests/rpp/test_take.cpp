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

#include <rpp/operators/take.hpp>
#include <rpp/sources/create.hpp>

#include "mock_observer.hpp"
#include "rpp/disposables/composite_disposable.hpp"

TEST_CASE("take operator limits emissions")
{
    int actually_values_sent{};
    auto obs = rpp::source::create<int>([&actually_values_sent](auto&& obs)
    {
        auto d = rpp::composite_disposable{};
        obs.set_upstream(d);

        while(!obs.is_disposed())
        {
            obs.on_next(actually_values_sent++);
        }

        CHECK(d.is_disposed());
        CHECK(obs.is_disposed());
    });

    mock_observer_strategy<int> mock{};


    SECTION("subscribe via take(3)")
    {
        (obs | rpp::operators::take(3)).subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{0, 1, 2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("subscribe via take(2) | take(3)")
    {
        (obs | rpp::operators::take(2) | rpp::operators::take(3)).subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{0, 1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("subscribe via take(0)")
    {
        (obs | rpp::operators::take(0)).subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector<int>{});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
 }

TEST_CASE("take operator forwards on_completed")
{
    auto obs = rpp::source::create<int>([](auto&& obs)
    {
        obs.on_completed();
    });

    mock_observer_strategy<int> mock{};
    (obs | rpp::operators::take(1)).subscribe(mock.get_observer());

    CHECK(mock.get_received_values() == std::vector<int>{});
    CHECK(mock.get_on_error_count() == 0);
    CHECK(mock.get_on_completed_count() == 1);
}

TEST_CASE("take operator forwards on_error")
{
    auto obs = rpp::source::create<int>([](auto&& obs)
    {
        obs.on_error({});
    });

    mock_observer_strategy<int> mock{};
    (obs | rpp::operators::take(1)).subscribe(mock.get_observer());

    CHECK(mock.get_received_values() == std::vector<int>{});
    CHECK(mock.get_on_error_count() == 1);
    CHECK(mock.get_on_completed_count() == 0);
}