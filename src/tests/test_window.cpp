//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <rpp/operators/window.hpp>
#include <rpp/operators/take.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include <rpp/sources/just.hpp>


SCENARIO("window subdivide observable into sub-observables", "[window]")
{
    GIVEN("observable of 3 items with window(2)")
    {
        auto obs = rpp::source::just(1,2,3).window(2);
        WHEN("subscribe on it")
        {
            THEN("see 2 observables")
            {
                auto mock = mock_observer<rpp::windowed_observable<int>>{};
                obs.subscribe(mock);

                CHECK(mock.get_total_on_next_count() == 2);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
            THEN("first windowed observable emits first 2 values and completes")
            {
                auto mock = mock_observer<int>{};
                size_t i = 0;
                obs.subscribe([&](const auto& observable)
                {
                    if (i++ == 0)
                        observable.subscribe(mock);
                });

                CHECK(mock.get_received_values() == std::vector{1,2});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
            THEN("second windowed observable emits last 1 value and completes")
            {
                auto mock = mock_observer<int>{};
                size_t i = 0;
                obs.subscribe([&](const auto& observable)
                    {
                        if (i++ == 1)
                            observable.subscribe(mock);
                    });

                CHECK(mock.get_received_values() == std::vector{ 3 });
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("subject of values")
    {
        auto subj = rpp::subjects::publish_subject<int>{};

        WHEN("subscribe on it via window(2)")
        {
            auto obs = subj.get_observable().window(2);
            AND_WHEN("emit first item")
            {
                auto mock = mock_observer<rpp::windowed_observable<int>>{};
                obs.subscribe(mock);

                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);

                subj.get_subscriber().on_next(1);

                THEN("see new windowed observable")
                {
                    CHECK(mock.get_total_on_next_count() == 1);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }

                AND_WHEN("emit second item")
                {
                    subj.get_subscriber().on_next(2);
                    THEN("no any new windowed observable")
                    {
                        CHECK(mock.get_total_on_next_count() == 1);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                    }

                    AND_WHEN("emit third item")
                    {
                        subj.get_subscriber().on_next(3);
                        THEN("new windowed observable")
                        {
                            CHECK(mock.get_total_on_next_count() == 2);
                            CHECK(mock.get_on_error_count() == 0);
                            CHECK(mock.get_on_completed_count() == 0);
                        }
                    }
                }
            }

            AND_WHEN("emit first item")
            {
                auto                          mock = mock_observer<int>{};
                obs.subscribe([&](const auto& observable)
                {
                    observable.subscribe(mock);
                });

                subj.get_subscriber().on_next(1);
                THEN("inner subscriber see first value without complete")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }

                AND_WHEN("emit on_completed")
                {
                    subj.get_subscriber().on_completed();

                    THEN("inner subscriber see completed")
                    {
                        CHECK(mock.get_received_values() == std::vector{ 1 });
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                    }
                }

                AND_WHEN("emit second item")
                {
                    subj.get_subscriber().on_next(2);

                    THEN("inner subscriber see second value and completes")
                    {
                        CHECK(mock.get_received_values() == std::vector{ 1,2 });
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                    }
                }
            }
        }
    }
}