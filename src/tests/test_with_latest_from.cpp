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

#include <rpp/operators/with_latest_from.hpp>
#include <rpp/operators/start_with.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/subjects/publish_subject.hpp>


TEST_CASE("with_latest_from combines observables")
{
    GIVEN("observables of the same type")
    {
        auto obs_1 = rpp::source::just(1);
        auto obs_2 = rpp::source::just(2);
        WHEN("subscribe on it via with_latest_from")
        {
            auto mock = mock_observer<std::tuple<int, int>>{};
            obs_1.with_latest_from(obs_2).subscribe(mock);
            THEN("obtain tuple of values")
            {
                CHECK(mock.get_received_values() == std::vector{ std::tuple{1, 2} });
                CHECK(mock.get_total_on_next_count() == 1);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via with_latest_from with custom selector")
        {
            auto                          mock = mock_observer<int>{};
            obs_1.with_latest_from([](int left, int right) { return left + right; }, obs_2).subscribe(mock);
            THEN("obtain values")
            {
                CHECK(mock.get_received_values() == std::vector{1+2});
                CHECK(mock.get_total_on_next_count() == 1);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

TEST_CASE("with_latest_from reacts only on main root but sends last value from others")
{
    GIVEN("subjects and subscribe on it with_latest_from")
    {
        auto subj_1 = rpp::subjects::publish_subject<int>{};
        auto subj_2 = rpp::subjects::publish_subject<int>{};
        auto mock = mock_observer<std::tuple<int, int>>{};
        subj_1.get_observable().with_latest_from(subj_2.get_observable()).subscribe(mock);
        WHEN("send only first subject sends value")
        {
            subj_1.get_subscriber().on_next(1);
            THEN("No values to observer")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
        WHEN("send only second subject sends value")
        {
            subj_2.get_subscriber().on_next(1);
            THEN("No values to observer")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
        WHEN("send first subject sends value then combine called with last second value")
        {
            subj_2.get_subscriber().on_next(1);
            subj_2.get_subscriber().on_next(2);
            subj_2.get_subscriber().on_next(3);
            subj_1.get_subscriber().on_next(4);
            THEN("obtain last combintaion")
            {
                CHECK(mock.get_received_values() == std::vector{ std::tuple{4, 3} });
                CHECK(mock.get_total_on_next_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
            AND_WHEN("second sends values again")
            {
                subj_2.get_subscriber().on_next(5);
                THEN("nothing new happens")
                {
                    CHECK(mock.get_received_values() == std::vector{ std::tuple{4, 3} });
                    CHECK(mock.get_total_on_next_count() == 1);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        WHEN("second completes")
        {
            subj_2.get_subscriber().on_completed();
            THEN("nothing happens")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
        WHEN("first completes")
        {
            subj_1.get_subscriber().on_completed();
            THEN("observer completed")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

SCENARIO("with_latest_from handles race condition", "[with_latest_from]")
{
    GIVEN("source observable in current thread pairs with error in other thread")
    {
        std::atomic_bool on_error_called{false};
        auto             subject = rpp::subjects::publish_subject<int>{};

        WHEN("subscribe on it")
        {
            THEN("on_error can't interleave with on_next")
            {
                std::thread th{};
                auto        source = rpp::subjects::publish_subject<int>{};

                source.get_observable()
                      .with_latest_from(subject.get_observable())
                      .subscribe([&](auto&&)
                                 {
                                     CHECK(!on_error_called);
                                     th = std::thread{[&]
                                     {
                                         subject.get_subscriber().on_error(std::exception_ptr{});
                                     }};
                                     std::this_thread::sleep_for(std::chrono::seconds{1});
                                     CHECK(!on_error_called);
                                 },
                                 [&](auto) { on_error_called = true; });

                subject.get_subscriber().on_next(2);
                source.get_subscriber().on_next(1);

                th.join();

                CHECK(on_error_called);
            }
        }
    }
}