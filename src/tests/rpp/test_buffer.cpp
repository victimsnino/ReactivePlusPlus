//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//                     TC Wang 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/flat_map.hpp>
#include <rpp/operators/buffer.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>

#include "mock_observer.hpp"

SCENARIO("buffer bundles items", "[buffer]")
{
    GIVEN("observable of -1-2-3-|")
    {
        auto mock = mock_observer<std::vector<int>>{};
        auto obs = rpp::source::just(1,2,3);
        WHEN("subscribe on it via buffer(0)")
        {
            obs.buffer(0)
                .subscribe(mock);
            THEN("shall see -{1}-{2}-{3}-|")
            {
                CHECK(mock.get_received_values() == std::vector{
                    std::vector{1},
                    std::vector{2},
                    std::vector{3}
                });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        WHEN("subscribe on it via buffer(1)")
        {
            obs.buffer(1)
                .subscribe(mock);
            THEN("shall see -{1}-{2}-{3}-|")
            {
                CHECK(mock.get_received_values() == std::vector{
                    std::vector{1},
                    std::vector{2},
                    std::vector{3}
                });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        WHEN("subscribe on it via buffer(2)")
        {
            obs.buffer(2)
                .subscribe(mock);
            THEN("shall see -{1,2}-{3}|")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                    std::vector{1,2},
                    std::vector{3},
                });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        WHEN("subscribe on it via buffer(3)")
        {
            obs.buffer(3)
                .subscribe(mock);
            THEN("shall see -{1,2,3}-|")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                    std::vector{1,2,3},
                });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
        WHEN("subscribe on it via buffer(4)")
        {
            obs.buffer(4)
                .subscribe(mock);
            THEN("shall see -{1,2,3}-|")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                        std::vector{1,2,3},
                });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
            }
        }
    }

    GIVEN("observable of -1-x-2-|, which error is raised in the middle")
    {
        auto obs = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                     rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})).as_dynamic(),
                                     rpp::source::just(2).as_dynamic())
                        .merge();
        auto mock = mock_observer<std::vector<int>>{};
        WHEN("subscribe on it via buffer(0)")
        {
            obs.buffer(0)
                .subscribe(mock);
            THEN("shall see -{1}-x, which means error event is through")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                    std::vector{1},
                });
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
            }
        }
        WHEN("subscribe on it via buffer(1)")
        {
            obs.buffer(1)
                .subscribe(mock);
            THEN("shall see -{1}-x, which means error event is through")
            {
                CHECK(mock.get_received_values() == std::vector<std::vector<int>>{
                    std::vector{1},
                });
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
            }
        }
        WHEN("subscribe on it via buffer(2)")
        {
            obs.buffer(2)
                .subscribe(mock);
            THEN("shall see --x, which means error event is through")
            {
                CHECK(mock.get_received_values().empty());
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
            }
        }
    }
}
