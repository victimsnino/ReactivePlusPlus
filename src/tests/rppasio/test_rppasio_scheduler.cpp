//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <catch2/catch_test_macros.hpp>

#include <rpp/observers/dynamic_observer.hpp>
#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/as_blocking.hpp>
#include <rpp/operators/flat_map.hpp>
#include <rpp/operators/subscribe_on.hpp>
#include <rpp/operators/tap.hpp>
#include <rpp/sources/just.hpp>

#include <asio/io_context.hpp>
#include <rppasio/schedulers/strand.hpp>

#include "rpp_trompeloil.hpp"

#include <chrono>

using namespace std::chrono_literals;

namespace
{
    void drain(const auto& disposable, asio::io_context& context)
    {
        while (!disposable.is_disposed())
        {
            context.run_one();
        }
    }
} // namespace

TEST_CASE("strand utilized current_thread")
{
    asio::io_context   context;
    mock_observer<int> mock;

    {
        auto worker = rppasio::schedulers::strand{context.get_executor()}.create_worker();
        auto obs    = mock.get_observer().as_dynamic();

        worker.schedule([](const auto& obs) {
            // As a strand can be executed on different threads it doesn't share queue with current_thread
            bool inner_schedule_executed = false;
            rpp::schedulers::current_thread::create_worker().schedule([&inner_schedule_executed](const auto&) {
                inner_schedule_executed = true;
                return rpp::schedulers::optional_delay_from_now{};
            },
                                                                      obs);

            CHECK(inner_schedule_executed);
            return rpp::schedulers::optional_delay_from_now{};
        },
                        obs);
    }

    context.run_one();
}

TEST_CASE("strand works till end")
{
    asio::io_context      context;
    mock_observer<int>    mock;
    trompeloeil::sequence seq;

    auto disposable = rpp::source::just(1, 2, 3)
                    | rpp::operators::subscribe_on(rppasio::schedulers::strand{context.get_executor()})
                    | rpp::operators::subscribe_with_disposable(mock.get_observer());

    REQUIRE_CALL(*mock, on_next_lvalue(1)).IN_SEQUENCE(seq);
    REQUIRE_CALL(*mock, on_next_lvalue(2)).IN_SEQUENCE(seq);
    REQUIRE_CALL(*mock, on_next_lvalue(3)).IN_SEQUENCE(seq);
    REQUIRE_CALL(*mock, on_completed()).IN_SEQUENCE(seq);

    drain(disposable, context);
}


TEST_CASE("strand in combination with current_thread")
{
    asio::io_context      context;
    mock_observer<int>    mock;
    trompeloeil::sequence seq;

    auto scheduler  = rppasio::schedulers::strand{context.get_executor()};
    auto disposable = rpp::source::just(scheduler, 1, 2, 3)
                    | rpp::ops::flat_map([](int v) {
                          return rpp::source::just(rpp::schedulers::current_thread{}, v);
                      })
                    | rpp::ops::subscribe_with_disposable(mock.get_observer());

    REQUIRE_CALL(*mock, on_next_lvalue(1)).IN_SEQUENCE(seq);
    REQUIRE_CALL(*mock, on_next_lvalue(2)).IN_SEQUENCE(seq);
    REQUIRE_CALL(*mock, on_next_lvalue(3)).IN_SEQUENCE(seq);
    REQUIRE_CALL(*mock, on_completed()).IN_SEQUENCE(seq);

    drain(disposable, context);
}

TEST_CASE("strand worker has correct execution order")
{
    asio::io_context context;
    auto             obs                 = mock_observer_strategy<int>{}.get_observer().as_dynamic();
    bool             has_run_first_task  = false;
    bool             has_run_second_task = false;

    auto worker = rppasio::schedulers::strand{context.get_executor()}.create_worker();

    worker.schedule(
        50ms,
        [&](const auto&) {
            has_run_first_task = true;
            return rpp::schedulers::optional_delay_from_now{};
        },
        obs);

    worker.schedule(
        5ms,
        [&](const auto&) {
            has_run_second_task = true;
            return rpp::schedulers::optional_delay_from_now{};
        },
        obs);

    context.run_one();
    CHECK_FALSE(has_run_first_task);
    CHECK(has_run_second_task);

    context.run_one();
    CHECK(has_run_first_task);
}