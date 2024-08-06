//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/element_at.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/just.hpp>

#include "copy_count_tracker.hpp"
#include "disposable_observable.hpp"

TEST_CASE("element_at emits element at index")
{
    mock_observer_strategy<int> mock{};

    SECTION("sequence of values")
    {
        auto obs = rpp::source::just(1, 2, 3);

        SECTION("subscribe via element_at(1)")
        {
            (obs | rpp::operators::element_at(1)).subscribe(mock);

            CHECK(mock.get_received_values() == std::vector{2});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }

        SECTION("subscribe via element_at(0)")
        {
            (obs | rpp::operators::element_at(0)).subscribe(mock);

            CHECK(mock.get_received_values() == std::vector{1});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }

        SECTION("subscribe via element_at(3)")
        {
            (obs | rpp::operators::element_at(3)).subscribe(mock);

            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_error_count() == 1);
            CHECK(mock.get_on_completed_count() == 0);
        }
    }

    SECTION("empty sequence")
    {
        auto obs = rpp::source::create<int>([](auto&& obs) {
            obs.on_completed();
        });

        SECTION("subscribe via element_at(0)")
        {
            (obs | rpp::operators::element_at(0)).subscribe(mock);

            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_error_count() == 1);
            CHECK(mock.get_on_completed_count() == 0);
        }
    }

    SECTION("error sequence")
    {
        auto obs = rpp::source::create<int>([](auto&& obs) {
            obs.on_error({});
        });

        SECTION("subscribe via element_at(0)")
        {
            (obs | rpp::operators::element_at(0)).subscribe(mock);

            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_error_count() == 1);
            CHECK(mock.get_on_completed_count() == 0);
        }
    }
}

TEST_CASE("element_at doesn't produce extra copies")
{
    SECTION("element_at(1)")
    {
        copy_count_tracker::test_operator(rpp::ops::element_at(1),
                                          {
                                              .send_by_copy = {.copy_count = 1, // 1 copy to final subscriber
                                                               .move_count = 0},
                                              .send_by_move = {.copy_count = 0,
                                                               .move_count = 1} // 1 move to final subscriber
                                          },
                                          2);
    }
}

TEST_CASE("element_at satisfies disposable contracts")
{
    test_operator_with_disposable<int>(rpp::ops::element_at(1));
}
