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

#include <rpp/operators/as_blocking.hpp>
#include <rpp/operators/combine_latest.hpp>
#include <rpp/schedulers/current_thread.hpp>
#include <rpp/schedulers/immediate.hpp>
#include <rpp/sources/concat.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/never.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include "disposable_observable.hpp"
#include "mock_observer.hpp"
#include "snitch_logging.hpp"

TEST_CASE("combine_latest bundles items")
{
    SECTION("observable of -1-2-3-| combines with -4-5-6-| on immediate scheduler")
    {
        auto mock = mock_observer_strategy<std::tuple<int, int>>{};
        rpp::source::just(rpp::schedulers::immediate{}, 1, 2, 3)
            | rpp::ops::combine_latest(rpp::source::just(rpp::schedulers::immediate{}, 4, 5, 6))
            | rpp::ops::subscribe(mock);

        CHECK(mock.get_received_values() == std::vector<std::tuple<int, int>>{
                  std::make_tuple(1, 6),
                  std::make_tuple(2, 6),
                  std::make_tuple(3, 6),
              });
        CHECK(mock.get_on_completed_count() == 1);
        CHECK(mock.get_on_error_count() == 0);
    }

    SECTION("observable of -1-2-3-| combines with -4-5-6-| on current_thread")
    {
        auto mock = mock_observer_strategy<std::tuple<int, int>>{};

        rpp::source::just(rpp::schedulers::current_thread{}, 1, 2, 3)                                 // source 1
            | rpp::ops::combine_latest(rpp::source::just(rpp::schedulers::current_thread{}, 4, 5, 6)) // source 2
            | rpp::ops::subscribe(mock);

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

    SECTION("observable of -1-2-3-| combines with two other sources on current_thread")
    {
        auto mock = mock_observer_strategy<std::tuple<int, int, int>>{};

        rpp::source::just(rpp::schedulers::current_thread{}, 1, 2, 3) // source 1
            | rpp::ops::combine_latest(
                rpp::source::just(rpp::schedulers::current_thread{}, 4, 5, 6), // source 2
                rpp::source::just(rpp::schedulers::current_thread{}, 7, 8, 9)) // source 3
            | rpp::ops::subscribe(mock);

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

TEST_CASE("combine_latest defers completed event")
{
    SECTION("observable of -1-2-3-| and never")
    {
        auto mock = mock_observer_strategy<std::tuple<int, int>>{};
        rpp::source::just(1, 2, 3)
            | rpp::ops::combine_latest(rpp::source::never<int>())
            | rpp::ops::subscribe(mock);

        CHECK(mock.get_received_values().empty());
        CHECK(mock.get_on_completed_count() == 0);
        CHECK(mock.get_on_error_count() == 0);
    }
}

TEST_CASE("combine_latest forwards errors")
{
    SECTION("observable of -1-2-3-| combines with error")
    {
        auto mock = mock_observer_strategy<std::tuple<int, int>>{};
        rpp::source::just(1, 2, 3)
            | rpp::ops::combine_latest(rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})))
            | rpp::ops::subscribe(mock);

        CHECK(mock.get_received_values().empty());
        CHECK(mock.get_on_completed_count() == 0);
        CHECK(mock.get_on_error_count() == 1);
    }
}

TEST_CASE("combine_latest handles race condition")
{
    SECTION("source observable in current thread pairs with error in other thread")
    {
        std::atomic_bool on_error_called{false};
        auto             subject = rpp::subjects::publish_subject<int>{};

        SECTION("subscribe on it")
        {
            SECTION("on_error can't interleave with on_next")
            {
                rpp::source::just(1, 1, 1)
                    | rpp::ops::combine_latest(rpp::source::concat(rpp::source::just(2), subject.get_observable()))
                    | rpp::ops::as_blocking()
                    | rpp::ops::subscribe([&](auto&&) {
                                       CHECK(!on_error_called);
                                       std::thread{[&]
                                       {
                                           subject.get_observer().on_error(std::exception_ptr{});
                                       }}.detach();
                                       std::this_thread::sleep_for(std::chrono::seconds{1});
                                       CHECK(!on_error_called); },
                                          [&](auto) { on_error_called = true; });

                CHECK(on_error_called);
            }
        }
    }
}

TEST_CASE("combine_latest satisfies disposable contracts")
{
    auto observable_disposable = rpp::composite_disposable_wrapper::make();
    {
        auto observable = observable_with_disposable<int>(observable_disposable);

        test_operator_with_disposable<int>(rpp::ops::combine_latest(observable));
    }

    CHECK(observable_disposable.is_disposed() || observable_disposable.lock().use_count() == 2);
}