//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/with_latest_from.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include "disposable_observable.hpp"


TEST_CASE("with_latest_from combines observables")
{
    auto obs_1 = rpp::source::just(1);
    auto obs_2 = rpp::source::just(2.2);
    SECTION("subscribe on it via with_latest_from")
    {
        auto mock = mock_observer_strategy<std::tuple<int, double>>{};
        obs_1 | rpp::ops::with_latest_from(obs_2) | rpp::ops::subscribe(mock);
        SECTION("obtain tuple of values")
        {
            CHECK(mock.get_received_values() == std::vector{std::tuple{1, 2.2}});
            CHECK(mock.get_total_on_next_count() == 1);
            CHECK(mock.get_on_completed_count() == 1);
        }
    }
    SECTION("subscribe on it via with_latest_from with custom selector")
    {
        auto mock = mock_observer_strategy<double>{};
        obs_1 | rpp::ops::with_latest_from([](int left, double right) { return left + right; }, obs_2) | rpp::ops::subscribe(mock);
        SECTION("obtain values")
        {
            CHECK(mock.get_received_values() == std::vector{1 + 2.2});
            CHECK(mock.get_total_on_next_count() == 1);
            CHECK(mock.get_on_completed_count() == 1);
        }
    }
}

TEST_CASE("with_latest_from reacts only on main root but sends last value from others")
{
    SECTION("subjects and subscribe on it with_latest_from")
    {
        auto subj_1 = rpp::subjects::publish_subject<int>{};
        auto subj_2 = rpp::subjects::publish_subject<int>{};
        auto mock   = mock_observer_strategy<std::tuple<int, int>>{};
        subj_1.get_observable() | rpp::ops::with_latest_from(subj_2.get_observable()) | rpp::ops::subscribe(mock);
        SECTION("send only first subject sends value")
        {
            subj_1.get_observer().on_next(1);
            SECTION("No values to observer")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
        SECTION("send only second subject sends value")
        {
            subj_2.get_observer().on_next(1);
            SECTION("No values to observer")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
        SECTION("send first subject sends value SECTION combine called with last second value")
        {
            subj_2.get_observer().on_next(1);
            subj_2.get_observer().on_next(2);
            subj_2.get_observer().on_next(3);
            subj_1.get_observer().on_next(4);
            SECTION("obtain last combintaion")
            {
                CHECK(mock.get_received_values() == std::vector{std::tuple{4, 3}});
                CHECK(mock.get_total_on_next_count() == 1);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
            SECTION("second sends values again")
            {
                subj_2.get_observer().on_next(5);
                SECTION("nothing new happens")
                {
                    CHECK(mock.get_received_values() == std::vector{std::tuple{4, 3}});
                    CHECK(mock.get_total_on_next_count() == 1);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        SECTION("second completes")
        {
            subj_2.get_observer().on_completed();
            SECTION("nothing happens")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
        SECTION("second errors")
        {
            subj_2.get_observer().on_error({});
            SECTION("error obtained")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
        SECTION("first completes")
        {
            subj_1.get_observer().on_completed();
            SECTION("observer completed")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        SECTION("first errors")
        {
            subj_1.get_observer().on_error({});
            SECTION("error obtained")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

TEST_CASE("with_latest_from handles race condition")
{
    SECTION("source observable in current thread pairs with error in other thread")
    {
        std::atomic_bool on_error_called{false};
        auto             subject = rpp::subjects::publish_subject<int>{};

        SECTION("subscribe on it")
        {
            SECTION("on_error can't interleave with on_next")
            {
                std::thread th{};
                auto        source = rpp::subjects::publish_subject<int>{};

                source.get_observable()
                    | rpp::ops::with_latest_from(subject.get_observable())
                    | rpp::ops::subscribe([&](auto&&) {
                                     CHECK(!on_error_called);
                                     th = std::thread{[&]
                                     {
                                         subject.get_observer().on_error(std::exception_ptr{});
                                     }};
                                     std::this_thread::sleep_for(std::chrono::seconds{1});
                                     CHECK(!on_error_called); },
                                          [&](auto) { on_error_called = true; });

                subject.get_observer().on_next(2);
                source.get_observer().on_next(1);

                th.join();

                CHECK(on_error_called);
            }
        }
    }
}


TEST_CASE("with_latest_from handles current_thread scheduling")
{
    auto mock = mock_observer_strategy<std::tuple<int, int>>{};

    rpp::source::just(rpp::schedulers::current_thread{}, 1, 2, 3)
        | rpp::operators::with_latest_from(rpp::source::just(rpp::schedulers::current_thread{}, 1, 2, 3))
        | rpp::operators::subscribe(mock);

    CHECK(mock.get_received_values() == std::vector{std::tuple{1, 1}, std::tuple{2, 2}, std::tuple{3, 3}});
    CHECK(mock.get_on_error_count() == 0);
    CHECK(mock.get_on_completed_count() == 1);
}

TEST_CASE("with_latest_from satisfies disposable contracts")
{
    auto observable_disposable = rpp::composite_disposable_wrapper::make();
    {
        auto observable = observable_with_disposable<int>(observable_disposable);
        auto op         = rpp::ops::with_latest_from(observable);

        test_operator_with_disposable<int>(op);
        test_operator_finish_before_dispose<int>(op);
    }

    CHECK((observable_disposable.is_disposed() || observable_disposable.lock().use_count() == 2));
}
