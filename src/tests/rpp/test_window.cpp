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

#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/operators/window.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include "mock_observer.hpp"
#include "disposable_observable.hpp"
#include "snitch_logging.hpp"

TEST_CASE("window subdivide observable into sub-observables")
{
    SECTION("observable of 3 items with window(2)")
    {
        auto obs = rpp::source::just(1,2,3) | rpp::ops::window(2);
        SECTION("subscribe on it")
        {
            SECTION("see 2 observables")
            {
                auto mock = mock_observer_strategy<rpp::window_observable<int>>{};
                obs.subscribe(mock);

                CHECK(mock.get_total_on_next_count() == 2);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
            SECTION("first window observable emits first 2 values and completes")
            {
                auto mock = mock_observer_strategy<int>{};
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
            SECTION("second window observable emits last 1 value and completes")
            {
                auto mock = mock_observer_strategy<int>{};
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

    SECTION("subject of values")
    {
        auto subj = rpp::subjects::publish_subject<int>{};

        SECTION("subscribe on it via window(2)")
        {
            auto obs = subj.get_observable() | rpp::ops::window(2);
            SECTION("emit first item")
            {
                auto mock = mock_observer_strategy<rpp::window_observable<int>>{};
                obs.subscribe(mock);

                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);

                subj.get_observer().on_next(1);

                SECTION("see new window observable")
                {
                    CHECK(mock.get_total_on_next_count() == 1);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }

                SECTION("emit second item")
                {
                    subj.get_observer().on_next(2);
                    SECTION("no any new window observable")
                    {
                        CHECK(mock.get_total_on_next_count() == 1);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                    }

                    SECTION("emit third item")
                    {
                        subj.get_observer().on_next(3);
                        SECTION("new window observable")
                        {
                            CHECK(mock.get_total_on_next_count() == 2);
                            CHECK(mock.get_on_error_count() == 0);
                            CHECK(mock.get_on_completed_count() == 0);
                        }
                    }
                }

                SECTION("emit on_error")
                {
                    subj.get_observer().on_error(std::make_exception_ptr(std::runtime_error{""}));
                    SECTION("subscriber see error")
                    {
                        CHECK(mock.get_total_on_next_count() == 1);
                        CHECK(mock.get_on_error_count() == 1);
                        CHECK(mock.get_on_completed_count() == 0);
                    }
                }
            }

            SECTION("emit first item")
            {
                auto mock = mock_observer_strategy<int>{};
                obs.subscribe([&](const auto& observable) { observable.subscribe(mock); },
                              [](const std::exception_ptr&) {});

                subj.get_observer().on_next(1);
                SECTION("inner subscriber see first value without complete")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }

                SECTION("emit on_completed")
                {
                    subj.get_observer().on_completed();

                    SECTION("inner subscriber see completed")
                    {
                        CHECK(mock.get_received_values() == std::vector{ 1 });
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                    }
                }

                SECTION("emit on_error")
                {
                    subj.get_observer().on_error(std::make_exception_ptr(std::runtime_error{""}));

                    SECTION("inner subscriber see error")
                    {
                        CHECK(mock.get_received_values() == std::vector{ 1 });
                        CHECK(mock.get_on_error_count() == 1);
                        CHECK(mock.get_on_completed_count() == 0);
                    }
                }

                SECTION("emit second item")
                {
                    subj.get_observer().on_next(2);

                    SECTION("inner subscriber see second value and completes")
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

TEST_CASE("window disposes original disposable only when everything is disposed")
{
    auto source_disposable = std::make_shared<rpp::composite_disposable>();
    auto obs = rpp::source::create<int>([source_disposable](auto&& obs)
    {
        obs.set_upstream(source_disposable);
        obs.on_next(1);
    });

    auto observer_disposable = std::make_shared<rpp::composite_disposable>();
    auto inner_observer_disposable = std::make_shared<rpp::composite_disposable>();
    obs 
        | rpp::ops::window(2) 
        | rpp::ops::subscribe(rpp::composite_disposable_wrapper{observer_disposable}, [inner_observer_disposable](const rpp::window_observable<int>& new_obs)
    {
        new_obs.subscribe(rpp::composite_disposable_wrapper{inner_observer_disposable}, [](int){});
    });

    CHECK(!source_disposable->is_disposed());
    CHECK(!observer_disposable->is_disposed());
    CHECK(!inner_observer_disposable->is_disposed());

    observer_disposable->dispose();

    CHECK(!source_disposable->is_disposed());
    CHECK(observer_disposable->is_disposed());
    CHECK(!inner_observer_disposable->is_disposed());

    inner_observer_disposable->dispose();

    CHECK(source_disposable->is_disposed());
    CHECK(observer_disposable->is_disposed());
    CHECK(inner_observer_disposable->is_disposed());
}

TEST_CASE("window satisfies disposable contracts")
{
    test_operator_with_disposable<int>(rpp::ops::window(1));
}