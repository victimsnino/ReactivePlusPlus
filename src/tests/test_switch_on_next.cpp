//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "copy_count_tracker.hpp"
#include "mock_observer.hpp"
#include "rpp/sources.hpp"

#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/switch_on_next.hpp>

#include <catch2/catch_test_macros.hpp>

SCENARIO("switch_on_next switches observable after obtaining new one", "[operators][switch_on_next]")
{
    auto mock = mock_observer<int>();
    GIVEN("just observable of just observables")
    {
        auto observable = rpp::source::just(rpp::source::just(1), rpp::source::just(2), rpp::source::just(3));
        WHEN("subscribe on it via switch_on_next")
        {
            observable.switch_on_next().subscribe(mock);
            THEN("obtains values as from concat")
            {
                CHECK(mock.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    GIVEN("just observable of just observables where second is error")
    {
        auto observable = rpp::source::just(rpp::source::just(1).as_dynamic(), 
                                            rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})).as_dynamic(), 
                                            rpp::source::just(3).as_dynamic());
        WHEN("subscribe on it via switch_on_next")
        {
            observable.switch_on_next().subscribe(mock);
            THEN("obtains values as from concat but stops on error")
            {
                CHECK(mock.get_received_values() == std::vector{1});
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
    GIVEN("just observable of just observables where second is completed")
    {
        auto observable = rpp::source::just(rpp::source::just(1).as_dynamic(), 
                                            rpp::source::empty<int>().as_dynamic(), 
                                            rpp::source::just(3).as_dynamic());
        WHEN("subscribe on it via switch_on_next")
        {
            observable.switch_on_next().subscribe(mock);
            THEN("obtains values as from concat")
            {
                CHECK(mock.get_received_values() == std::vector{1,3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    GIVEN("just observable of just observables where second is never")
    {
        auto observable = rpp::source::just(rpp::source::just(1).as_dynamic(), 
                                            rpp::source::never<int>().as_dynamic(), 
                                            rpp::source::just(3).as_dynamic());
        WHEN("subscribe on it via switch_on_next")
        {
            observable.switch_on_next().subscribe(mock);
            THEN("obtains values as from concat")
            {
                CHECK(mock.get_received_values() == std::vector{1,3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

SCENARIO("switch_on_next doesn't produce extra copies", "[operators][switch_on_next][track_copy]")
{
    GIVEN("observable and subscriber")
    {
        copy_count_tracker verifier{};
        auto          obs = rpp::source::just(verifier.get_observable()).switch_on_next();
        WHEN("subscribe")
        {
            obs.subscribe([](copy_count_tracker){});
            THEN("no extra copies")
            {
                REQUIRE(verifier.get_copy_count() == 1); // 1 copy to final lambda
                REQUIRE(verifier.get_move_count() == 0);
            }
        }
    }
}


SCENARIO("switch_on_next doesn't produce extra copies for move", "[operators][switch_on_next][track_copy]")
{
    GIVEN("observable and subscriber")
    {
        copy_count_tracker verifier{};
        auto          obs = rpp::source::just(verifier.get_observable_for_move()).switch_on_next();
        WHEN("subscribe")
        {
            obs.subscribe([](copy_count_tracker){});
            THEN("no extra copies")
            {
                REQUIRE(verifier.get_copy_count() == 0); 
                REQUIRE(verifier.get_move_count() == 1); // 1 move to final lambda
            }
        }
    }
}