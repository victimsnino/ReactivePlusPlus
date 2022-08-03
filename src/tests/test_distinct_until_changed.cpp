//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "copy_count_tracker.hpp"

#include <rpp/operators/distinct_until_changed.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include <catch2/catch_test_macros.hpp>

#include "mock_observer.hpp"

SCENARIO("distinct_until_changed filters out consecutive duplicates and send first value from duplicates", "[distinct_until_changed]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable of values with duplicates")
    {
        auto obs = rpp::source::just(1, 1, 2, 2, 3, 2, 2, 1);
        WHEN("subscribe on it via distinct_until_changed")
        {
            obs.distinct_until_changed().subscribe(mock);
            THEN("subscriber obtains values without consecutive duplicates")
            {
                CHECK(mock.get_received_values() == std::vector{ 1,2,3,2,1 });
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via distinct_until_changed with custom comparator")
        {
            obs.distinct_until_changed([](int old_value, int new_value) {return old_value + new_value == 3; }).subscribe(mock);
            THEN("subscriber obtains values without consecutive duplicates")
            {
                CHECK(mock.get_received_values() == std::vector{ 1, 1, 3, 2, 2});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("subject of values")
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        WHEN("subscribe on it via distinct_until_changed and send value")
        {
            subj.get_observable().distinct_until_changed().subscribe(mock);
            subj.get_subscriber().on_next(1);
            THEN("subscriber obtains value")
            {
                CHECK(mock.get_received_values() == std::vector{ 1});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
            AND_WHEN("send duplicate")
            {
                subj.get_subscriber().on_next(1);
                THEN("subscriber ignores duplicated value")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1 });
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
    }
}

SCENARIO("distinct_until_changed keeps state for copies", "[distinct_until_changed]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable which sends values via copy")
    {
        auto obs = rpp::source::create<int>([](const auto& sub)
        {
            for(size_t i = 0; i < 10; ++i)
            {
                auto copy = sub;
                copy.on_next(1);
            }
        });
        WHEN("subscribe on it via distinct_until_changed")
        {
            obs.distinct_until_changed().subscribe(mock);
            THEN("observer obtains values as expected")
            {
                CHECK(mock.get_received_values() == std::vector{ 1 });
            }
        }
    }
}

SCENARIO("distinct_until_changed doesn't produce extra copies", "[distinct_until_changed][track_copy]")
{
    GIVEN("observable and subscriber")
    {
        copy_count_tracker verifier{};
        auto               obs = verifier.get_observable().distinct_until_changed();
        WHEN("subscribe")
        {
            obs.subscribe([](copy_count_tracker){});
            THEN("no extra copies")
            {
                REQUIRE(verifier.get_copy_count() == 2); // 1 copy to internal state + 1 copy to final subscriber
                REQUIRE(verifier.get_move_count() == 0); 
            }
        }
    }
}

SCENARIO("distinct_until_changed doesn't produce extra copies for move", "[distinct_until_changed][track_copy]")
{
    GIVEN("observable and subscriber")
    {
        copy_count_tracker verifier{};
        auto               obs = verifier.get_observable_for_move().distinct_until_changed();
        WHEN("subscribe")
        {
            obs.subscribe([](copy_count_tracker) {});
            THEN("no extra copies")
            {
                REQUIRE(verifier.get_copy_count() == 1); // 1 copy to internal state
                REQUIRE(verifier.get_move_count() == 1); // 1 move to final subscriber
            }
        }
    }
}