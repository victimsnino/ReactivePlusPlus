//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"
#include "rpp/disposables/fwd.hpp"
#include "rpp/operators/fwd.hpp"
#include "rpp/operators/subscribe.hpp"
#include <snitch/snitch.hpp>
#include <rpp/observables.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/sources/never.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/operators/as_blocking.hpp>
#include <chrono>
#include <thread>

TEST_CASE("create observable works properly as observable")
{
    size_t on_subscribe_called{};
    auto observable = rpp::source::create<int>([&](auto&& observer)
    {
        ++on_subscribe_called;
        observer.on_next(1);
        observer.on_completed();
    });

    auto test = [&](auto&& observable)
    {
        SECTION("subscribe valid observer")
        {
            std::vector<int> on_next_vals{};
            size_t           on_error{};
            size_t           on_completed{};

            observable.subscribe([&](int v) { on_next_vals.push_back(v); },
                                [&](const std::exception_ptr&) { ++on_error; },
                                [&]() { ++on_completed; });

            CHECK(on_subscribe_called == 1u);
            CHECK(on_next_vals == std::vector{1});
            CHECK(on_error == 0u);
            CHECK(on_completed == 1u);
        }

        SECTION("subscribe disposed callbacks")
        {
            observable.subscribe(rpp::disposable_wrapper{}, [](int) {}, [](const std::exception_ptr&) {}, []() {});

            CHECK(on_subscribe_called == 0u);
        }

        SECTION("subscribe disposed observer")
        {
            observable.subscribe(rpp::disposable_wrapper{}, rpp::make_lambda_observer([](int) {}, [](const std::exception_ptr&) {}, []() {}));

            CHECK(on_subscribe_called == 0u);
        }

        SECTION("subscribe non-disposed callbacks")
        {
            observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, [](int) {}, [](const std::exception_ptr&) {}, []() {});

            CHECK(on_subscribe_called == 1u);
        }

        SECTION("subscribe non-disposed observer")
        {
            observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, rpp::make_lambda_observer([](int) {}, [](const std::exception_ptr&) {}, []() {}));

            CHECK(on_subscribe_called == 1u);
        }
    };

    SECTION("original observable")
    {
        test(observable);
    }

    SECTION("dynamic observable")
    {
        test(observable.as_dynamic());
    }

    SECTION("dynamic observable via move")
    {
        test(std::move(observable).as_dynamic()); // NOLINT
    }
}

TEST_CASE("blocking_observable blocks subscribe call")
{
    mock_observer_strategy<int> mock{};
    SECTION("on_completed inside observable")
    {
        rpp::source::create<int>([](auto&& observer)
        {
            std::thread(
                [observer=std::forward<decltype(observer)>(observer)]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds{100});
                    observer.on_completed();
                })
                .detach();
        })
        | rpp::operators::as_blocking()
        | rpp::operators::subscribe(mock.get_observer().as_dynamic());

        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("on_error inside observable")
    {
        auto op = rpp::operators::as_blocking();
        auto obs =  rpp::source::create<int>([](auto&& observer)
        {
            std::thread(
                [observer=std::forward<decltype(observer)>(observer)]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds{100});
                    observer.on_error({});
                })
                .detach();
        });

        obs
        | op
        | rpp::operators::subscribe(mock.get_observer().as_dynamic());

        CHECK(mock.get_on_error_count() == 1);
    }
}

TEST_CASE("base observables")
{
    mock_observer_strategy<int> mock{ };

    SECTION("empty")
    {
        auto observable = rpp::source::empty<int>();
        SECTION("subscribe on this observable")
        {
            observable.subscribe(mock.get_observer());
            SECTION("only on_completed called")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    SECTION("never")
    {
        auto observable = rpp::source::never<int>();
        SECTION("subscribe on this observable")
        {
            observable.subscribe(mock.get_observer());
            SECTION("no any callbacks")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
    SECTION("error")
    {
        auto observable = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{"MY EXCEPTION"}));
        SECTION("subscribe on this observable")
        {
            observable.subscribe(mock.get_observer());
            SECTION("only on_error callback once")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}
