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

#include <rpp/operators/timeout.hpp>
#include <rpp/operators/delay.hpp>
#include <rpp/sources/just.hpp>

#include "snitch_logging.hpp"

#include "mock_observer.hpp"
#include "test_scheduler.hpp"

TEST_CASE("timeout subscribes to passed observable in case of reaching timeout")
{
    auto       scheduler = test_scheduler{};
    auto       mock      = mock_observer_strategy<int>{};
    const auto now       = scheduler.now();

    SECTION("timeout not reached")
    {
        rpp::source::just(scheduler, 1)
            | rpp::ops::timeout(std::chrono::seconds{1}, rpp::source::just(2), scheduler)
            | rpp::ops::subscribe(mock);

        scheduler.time_advance(std::chrono::seconds{0});
        scheduler.time_advance(std::chrono::seconds{1});
        scheduler.time_advance(std::chrono::seconds{1});

        CHECK(scheduler.get_executions() == std::vector{now});
        CHECK(scheduler.get_schedulings() == std::vector{now + std::chrono::seconds{1}, now});
        CHECK(mock.get_received_values() == std::vector{1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("timeout reached")
    {
        rpp::source::just(scheduler, 1)
            | rpp::ops::delay(std::chrono::seconds{5}, scheduler)
            | rpp::ops::timeout(std::chrono::seconds{1}, rpp::source::just(2), scheduler)
            | rpp::ops::subscribe(mock);
        
        scheduler.time_advance(std::chrono::seconds{0});
        
        for (size_t i =0; i < 10; ++i)
            scheduler.time_advance(std::chrono::seconds{1});

        CHECK(scheduler.get_executions() == std::vector{now, now + std::chrono::seconds{1}});
        CHECK(scheduler.get_schedulings() == std::vector{now + std::chrono::seconds{1}, now, now + std::chrono::seconds{5}});
        CHECK(mock.get_received_values() == std::vector{2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
}