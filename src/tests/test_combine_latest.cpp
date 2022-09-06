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

#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/combine_latest.hpp>
#include <rpp/schedulers/new_thread_scheduler.hpp>
#include <rpp/schedulers/trampoline_scheduler.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/interval.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/never.hpp>

#include <chrono>

#include "mock_observer.hpp"

SCENARIO("combine_latest bundles items", "[combine_latest]")
{
    GIVEN("observable of -1-2-3-| combines with -4-5-6-| on default scheduler")
    {
        auto mock = mock_observer<std::tuple<int, int>>{};
        rpp::source::just(1, 2, 3)
                .combine_latest(rpp::source::just(4, 5, 6))
                .subscribe(mock);

        CHECK(mock.get_received_values() == std::vector<std::tuple<int, int>>{
                std::make_tuple(1, 6),
                std::make_tuple(2, 6),
                std::make_tuple(3, 6),
        });
        CHECK(mock.get_on_completed_count() == 1);
        CHECK(mock.get_on_error_count() == 0);
    }

    GIVEN("observable of -1-2-3-| combines with -4-5-6-| on trampoline")
    {
        auto mock = mock_observer<std::tuple<int, int>>{};

        rpp::source::just(rpp::schedulers::trampoline{}, 1, 2, 3)                      // source 1
            .combine_latest(rpp::source::just(rpp::schedulers::trampoline{}, 4, 5, 6)) // source 2
            .subscribe(mock);

        // Above stream should output in such sequence
        // source 1:   -1---2---3-|
        // source 2: -4---5---6-|

        CHECK(mock.get_received_values() == std::vector<std::tuple<int, int>>{
            std::make_tuple(1, 4),
            std::make_tuple(1, 5),
            std::make_tuple(2, 5),
            std::make_tuple(2, 6),
            std::make_tuple(3, 6),
        });
        CHECK(mock.get_on_completed_count() == 1);
        CHECK(mock.get_on_error_count() == 0);
    }

    GIVEN("observable of -1-2-3-| combines with two other sources on trampoline")
    {
        auto mock = mock_observer<std::tuple<int, int, int>>{};

        rpp::source::just(rpp::schedulers::trampoline{}, 1, 2, 3)          // source 1
            .combine_latest(
                rpp::source::just(rpp::schedulers::trampoline{}, 4, 5, 6), // source 2
                rpp::source::just(rpp::schedulers::trampoline{}, 7, 8 ,9)) // source 3
            .subscribe(mock);

        // Above stream should output in such sequence
        // source 1:   --1---2---3-|
        // source 2: -4---5---6-|
        // source 3: --7---8---9-|

        CHECK(mock.get_received_values() == std::vector<std::tuple<int, int, int>>{
            std::make_tuple(1, 4, 7),
            std::make_tuple(1, 5, 7),
            std::make_tuple(1, 5, 8),
            std::make_tuple(2, 5, 8),
            std::make_tuple(2, 6, 8),
            std::make_tuple(2, 6, 9),
            std::make_tuple(3, 6, 9),
        });
        CHECK(mock.get_on_completed_count() == 1);
        CHECK(mock.get_on_error_count() == 0);
    }
}

SCENARIO("combine_latest defers completed event", "[combine_latest]")
{
    GIVEN("observable of -1-2-3-| and never")
    {
        auto mock = mock_observer<std::tuple<int, int>>{};
        rpp::source::just(1, 2, 3)
            .combine_latest(rpp::source::never<int>())
            .subscribe(mock);

        CHECK(mock.get_received_values().empty());
        CHECK(mock.get_on_completed_count() == 0);
        CHECK(mock.get_on_error_count() == 0);
    }
}

SCENARIO("combine_latest forwards errors", "[combine_latest]")
{
    GIVEN("observable of -1-2-3-| combines with error")
    {
        auto mock = mock_observer<std::tuple<int, int>>{};
        rpp::source::just(1, 2, 3)
            .combine_latest(rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})))
            .subscribe(mock);

        CHECK(mock.get_received_values().empty());
        CHECK(mock.get_on_completed_count() == 0);
        CHECK(mock.get_on_error_count() == 1);
    }
}

SCENARIO("combine_latest handles race condition", "[combine_latest]")
{
    GIVEN("source observable in current thread pairs with error in other thread")
    {
        std::atomic_bool on_error_called{false};

        rpp::source::interval(std::chrono::seconds{1}, rpp::schedulers::trampoline{})
            .combine_latest(rpp::source::interval(std::chrono::seconds{1}, rpp::schedulers::new_thread{}),
                            rpp::source::interval(std::chrono::seconds{2}, rpp::schedulers::new_thread{})
                    .map([](const auto& count) -> size_t
                    {
                        // We wait until a successful composition then throw error
                        if (count < 1)
                            return count;
                        throw std::runtime_error{""};
                    }))
            .as_blocking()
            .subscribe([&](auto &&)
                       {
                           CHECK(!on_error_called);
                           std::this_thread::sleep_for(std::chrono::seconds{3});
                           CHECK(!on_error_called);
                       },
                       [&](const std::exception_ptr&)
                       {
                           on_error_called = true;
                       });

        CHECK(on_error_called);
    }
}