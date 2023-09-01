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
#include <rpp/operators/subscribe.hpp>
#include <rpp/operators/take.hpp>
#include <rpp/sources/interval.hpp>
#include <rpp/schedulers/current_thread.hpp>
#include <rpp/schedulers/immediate.hpp>
#include <rpp/schedulers/new_thread.hpp>

#include "mock_observer.hpp"

#include <chrono>

using namespace std::chrono_literals;

TEMPLATE_TEST_CASE("interval emits a sequential integer every time interval",
                   "",
                   rpp::schedulers::current_thread,
                   rpp::schedulers::immediate,
                   rpp::schedulers::new_thread)
{
    using scheduler = TestType;

    SECTION("no initial duration")
    {
        auto mock = mock_observer_strategy<size_t>();
        rpp::source::interval(10ms, scheduler{})
            | rpp::operators::take(5)
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector<size_t>{1, 2, 3, 4, 5});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("with initial duration")
    {
        auto mock = mock_observer_strategy<size_t>();
        rpp::source::interval(5ms, 10ms, scheduler{})
            | rpp::operators::take(5)
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector<size_t>{1, 2, 3, 4, 5});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
}