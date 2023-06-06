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
#include <rpp/operators/subscribe_on.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/create.hpp>

#include <rpp/schedulers/new_thread.hpp>
#include <thread>

#include "mock_observer.hpp"

TEST_CASE("subscribe_on schedules job in another scheduler")
{
    auto mock = mock_observer_strategy<int>{};
    auto scheduler = rpp::schedulers::new_thread{};

    SECTION("observable")
    {
        std::promise<std::thread::id> thread_id{};
        auto obs = rpp::source::create<int>([&](const auto& sub)
        {
            thread_id.set_value(std::this_thread::get_id());
            sub.on_next(1);
            sub.on_completed();
        });
        SECTION("subscribe on it with subscribe_on")
        {
            obs | rpp::ops::subscribe_on(scheduler) | rpp::ops::as_blocking() | rpp::ops::subscribe(mock.get_observer());
            SECTION("expect to obtain value via scheduling")
            {
                REQUIRE(mock.get_total_on_next_count() == 1);
                REQUIRE(mock.get_on_completed_count() == 1);
                REQUIRE(thread_id.get_future().get() != std::this_thread::get_id());
            }
        }
    }
    SECTION("observable with error")
    {
        auto obs = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));
        SECTION("subscribe on it with subscribe_on")
        {
            obs | rpp::ops::subscribe_on(scheduler) | rpp::ops::as_blocking() | rpp::ops::subscribe(mock.get_observer());
            SECTION("expect to obtain error via scheduling")
            {
                REQUIRE(mock.get_total_on_next_count() == 0);
                REQUIRE(mock.get_on_error_count() == 1);
                REQUIRE(mock.get_on_completed_count() == 0);
            }
        }
    }
}
