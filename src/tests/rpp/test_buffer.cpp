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

#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/buffer.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/just.hpp>

#include "disposable_observable.hpp"
#include "mock_observer.hpp"

TEST_CASE("buffer bundles items")
{
    SECTION("observable of -1-2-3-|")
    {
        auto mock = mock_observer_strategy<std::vector<int>>{};
        auto obs  = rpp::source::just(1, 2, 3);
        SECTION("subscribe on it via buffer(0)")
        {
            obs | rpp::ops::buffer(0)
                | rpp::ops::subscribe(mock);
            SECTION("shall see -{1}-{2}-{3}-|")
            {
                CHECK(mock.get_received_values() == std::vector{std::vector{1}, std::vector{2}, std::vector{3}});
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        SECTION("subscribe on it via buffer(1)")
        {
            obs | rpp::ops::buffer(1)
                | rpp::ops::subscribe(mock);
            SECTION("shall see -{1}-{2}-{3}-|")
            {
                CHECK(mock.get_received_values() == std::vector{std::vector{1}, std::vector{2}, std::vector{3}});
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        SECTION("subscribe on it via buffer(2)")
        {
            obs | rpp::ops::buffer(2)
                | rpp::ops::subscribe(mock);
            SECTION("shall see -{1,2}-{3}|")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                          std::vector{1, 2},
                          std::vector{3},
                      });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        SECTION("subscribe on it via buffer(3)")
        {
            obs | rpp::ops::buffer(3)
                | rpp::ops::subscribe(mock);
            SECTION("shall see -{1,2,3}-|")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                          std::vector{1, 2, 3},
                      });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        SECTION("subscribe on it via buffer(4)")
        {
            obs | rpp::ops::buffer(4)
                | rpp::ops::subscribe(mock);
            SECTION("shall see -{1,2,3}-|")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                          std::vector{1, 2, 3},
                      });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
    }

    SECTION("observable of -1-x-2-|, which error is raised in the middle")
    {
        auto obs = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                     rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})).as_dynamic(),
                                     rpp::source::just(2).as_dynamic())
                 | rpp::ops::merge();
        auto mock = mock_observer_strategy<std::vector<int>>{};
        SECTION("subscribe on it via buffer(0)")
        {
            obs | rpp::ops::buffer(0)
                | rpp::ops::subscribe(mock);
            SECTION("shall see -{1}-x, which means error event is through")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                          std::vector{1},
                      });
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
            }
        }
        SECTION("subscribe on it via buffer(1)")
        {
            obs | rpp::ops::buffer(1)
                | rpp::ops::subscribe(mock);
            SECTION("shall see -{1}-x, which means error event is through")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                          std::vector{1},
                      });
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
            }
        }
        SECTION("subscribe on it via buffer(2)")
        {
            obs | rpp::ops::buffer(2)
                | rpp::ops::subscribe(mock);
            SECTION("shall see --x, which means error event is through")
            {
                CHECK(mock.get_received_values().empty());
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
            }
        }
    }
}

TEST_CASE("buffer satisfies disposable contracts")
{
    test_operator_with_disposable<int>(rpp::ops::buffer(1));
}