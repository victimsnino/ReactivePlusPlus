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

#include <rpp/operators/throttle.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include "mock_observer.hpp"
#include "disposable_observable.hpp"

#include "test_scheduler.hpp"
#include "snitch_logging.hpp"

TEST_CASE("throttle throttles emissions")
{
    auto mock = mock_observer_strategy<std::tuple<int, rpp::schedulers::time_point>>{};
    auto subj = rpp::subjects::publish_subject<int>{};
    const auto throttle_duration = std::chrono::seconds{2};
    subj.get_observable() | rpp::ops::throttle<test_scheduler>(throttle_duration) | rpp::ops::map([](int v){return std::tuple{v, test_scheduler::now()};}) | rpp::ops::subscribe(mock);
    SECTION("emiting second value forwards it immediately")
    {
        const auto first_value_time = test_scheduler::now();
        subj.get_observer().on_next(1);
        CHECK(mock.get_received_values() == std::vector{std::tuple{1, first_value_time}});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
        SECTION("emitting second value in throttle_duration/2 not forwards it")
        {
            test_scheduler{}.time_advance(throttle_duration/2);

            subj.get_observer().on_next(2);
            CHECK(mock.get_received_values() == std::vector{std::tuple{1, first_value_time}});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 0);
        }
        SECTION("emitting error forwards it immediately")
        {
            subj.get_observer().on_error({});
            CHECK(mock.get_received_values() == std::vector{std::tuple{1, first_value_time}});
            CHECK(mock.get_on_error_count() == 1);
            CHECK(mock.get_on_completed_count() == 0);
        }
        SECTION("emitting completed forwards it immediately")
        {
            subj.get_observer().on_completed();
            CHECK(mock.get_received_values() == std::vector{std::tuple{1, first_value_time}});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
        SECTION("emitting second value in throttle_duration forwards it")
        {
            test_scheduler{}.time_advance(throttle_duration);

            subj.get_observer().on_next(2);
            CHECK(mock.get_received_values() == std::vector{std::tuple{1, first_value_time}, std::tuple{2, test_scheduler::now()}});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 0);
        }
        SECTION("emitting second value in 3/2*throttle_duration forwards it")
        {
            test_scheduler{}.time_advance(throttle_duration/2*3);

            subj.get_observer().on_next(2);
            CHECK(mock.get_received_values() == std::vector{std::tuple{1, first_value_time}, std::tuple{2, test_scheduler::now()}});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 0);
        }
    }
}


TEST_CASE("throttle satisfies disposable contracts")
{
    test_operator_with_disposable<int>(rpp::ops::throttle(std::chrono::seconds{1}));
}