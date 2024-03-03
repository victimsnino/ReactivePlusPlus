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

#include <rpp/operators/delay.hpp>
#include <rpp/operators/timeout.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/never.hpp>

#include "disposable_observable.hpp"
#include "mock_observer.hpp"
#include "snitch_logging.hpp"
#include "test_scheduler.hpp"


TEST_CASE("timeout subscribes to passed observable in case of reaching timeout")
{
    auto       scheduler = test_scheduler{};
    auto       mock      = mock_observer_strategy<int>{};
    const auto now       = scheduler.now();

    SECTION("timeout not reached")
    {
        rpp::source::just(scheduler, 1, 2, 3)
            | rpp::ops::timeout(std::chrono::seconds{1}, rpp::source::just(100), scheduler)
            | rpp::ops::subscribe(mock);

        for (size_t i = 0; i < 3; ++i)
            scheduler.time_advance(std::chrono::seconds{0});
        scheduler.time_advance(std::chrono::seconds{1});

        CHECK(scheduler.get_executions() == std::vector{now, now, now});
        CHECK(scheduler.get_schedulings() == std::vector{now + std::chrono::seconds{1}, now, now, now});
        CHECK(mock.get_received_values() == std::vector{1, 2, 3});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("timeout reached")
    {
        rpp::source::just(scheduler, 1)
            | rpp::ops::delay(std::chrono::seconds{5}, scheduler)
            | rpp::ops::timeout(std::chrono::seconds{1}, rpp::source::just(100), scheduler)
            | rpp::ops::subscribe(mock);

        scheduler.time_advance(std::chrono::seconds{0});

        for (size_t i = 0; i < 10; ++i)
            scheduler.time_advance(std::chrono::seconds{1});

        CHECK(scheduler.get_executions() == std::vector{now, now + std::chrono::seconds{1}});
        CHECK(scheduler.get_schedulings() == std::vector{now + std::chrono::seconds{1}, now, now + std::chrono::seconds{5}});
        CHECK(mock.get_received_values() == std::vector{100});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("timeout with never")
    {
        rpp::source::never<int>()
            | rpp::ops::timeout(std::chrono::seconds{1}, rpp::source::just(100), scheduler)
            | rpp::ops::subscribe(mock);

        scheduler.time_advance(std::chrono::seconds{1});

        CHECK(scheduler.get_executions() == std::vector{now + std::chrono::seconds{1}});
        CHECK(scheduler.get_schedulings() == std::vector{now + std::chrono::seconds{1}});
        CHECK(mock.get_received_values() == std::vector{100});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("timeout with empty")
    {
        rpp::source::empty<int>()
            | rpp::ops::timeout(std::chrono::seconds{1}, rpp::source::just(100), scheduler)
            | rpp::ops::subscribe(mock);

        scheduler.time_advance(std::chrono::seconds{1});

        CHECK(scheduler.get_executions() == std::vector<rpp::schedulers::time_point>{});
        CHECK(scheduler.get_schedulings() == std::vector{now + std::chrono::seconds{1}});
        CHECK(mock.get_received_values() == std::vector<int>{});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("timeout with error")
    {
        rpp::source::error<int>({})
            | rpp::ops::timeout(std::chrono::seconds{1}, rpp::source::just(100), scheduler)
            | rpp::ops::subscribe(mock);

        scheduler.time_advance(std::chrono::seconds{1});

        CHECK(scheduler.get_executions() == std::vector<rpp::schedulers::time_point>{});
        CHECK(scheduler.get_schedulings() == std::vector{now + std::chrono::seconds{1}});
        CHECK(mock.get_received_values() == std::vector<int>{});
        CHECK(mock.get_on_error_count() == 1);
        CHECK(mock.get_on_completed_count() == 0);
    }

    SECTION("timeout with default args")
    {
        rpp::source::never<int>()
            | rpp::ops::timeout(std::chrono::seconds{1}, scheduler)
            | rpp::ops::subscribe(mock);

        scheduler.time_advance(std::chrono::seconds{1});

        CHECK(scheduler.get_executions() == std::vector{now + std::chrono::seconds{1}});
        CHECK(scheduler.get_schedulings() == std::vector{now + std::chrono::seconds{1}});
        CHECK(mock.get_received_values() == std::vector<int>{});
        CHECK(mock.get_on_error_count() == 1);
        CHECK(mock.get_on_completed_count() == 0);
    }

    SECTION("never timeout with never")
    {
        rpp::source::never<int>()
            | rpp::ops::timeout(std::chrono::seconds{1}, rpp::source::never<int>(), scheduler)
            | rpp::ops::subscribe(mock);

        scheduler.time_advance(std::chrono::seconds{1});

        CHECK(scheduler.get_executions() == std::vector{now + std::chrono::seconds{1}});
        CHECK(scheduler.get_schedulings() == std::vector{now + std::chrono::seconds{1}});
        CHECK(mock.get_received_values() == std::vector<int>{});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
    }
}

TEST_CASE("timeout satisfies disposable contracts")
{
    test_operator_with_disposable<int>(rpp::ops::timeout(std::chrono::seconds{10000000}, test_scheduler{}));
}