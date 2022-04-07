// MIT License
// 
// Copyright (c) 2021 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "copy_count_tracker.h"
#include "mock_observer.h"
#include "rpp/schedulers/new_thread_scheduler.h"

#include <catch2/catch_test_macros.hpp>

#include <rpp/sources/create.h>
#include <rpp/sources/just.h>

#include <rpp/observables.h>
#include <rpp/observers.h>
#include <rpp/subscribers.h>
#include <rpp/observables/dynamic_observable.h>

#include <array>
#include <future>

SCENARIO("Any observable can be casted to dynamic_observable", "[observable]")
{
    auto validate_observable =[](const auto& observable)
    {
        WHEN("Call as_dynamic function")
        {
            auto dynamic_observable = observable.as_dynamic();

            THEN("Obtain dynamic_observable of same type")
                static_assert(std::is_same<decltype(dynamic_observable), rpp::dynamic_observable<int>>{}, "Type of dynamic observable should be same!");
        }

        WHEN("Construct dynamic_observable by constructor")
        {
            auto dynamic_observable = rpp::dynamic_observable{observable};

            THEN("Obtain dynamic_observable of same type")
                static_assert(std::is_same<decltype(dynamic_observable), rpp::dynamic_observable<int>>{}, "Type of dynamic observable should be same!");
        }
    };

    GIVEN("specific_observable")
        validate_observable(rpp::specific_observable([](const rpp::dynamic_subscriber<int>&) {}));

    GIVEN("dynamic_observable")
        validate_observable(rpp::dynamic_observable([](const rpp::dynamic_subscriber<int>&) {}));
}

SCENARIO("Any observable can be subscribed from any type of subscription", "[observable]")
{
    int  subscribe_count     = 0;
    auto validate_observable = [&](const auto& observable)
    {
        auto validate_subscribe = [&](auto&&...args)
        {
            observable.subscribe(std::forward<decltype(args)>(args)...);
            THEN("subscribe called")
                REQUIRE(subscribe_count == 1);
        };
        WHEN("subscribe with no arguments")
            validate_subscribe();
        WHEN("subscribe with lambda with specified type")
            validate_subscribe([](const int&){});
        WHEN("subscribe with lambda with specified type + error")
            validate_subscribe([](const int&){}, [](const std::exception_ptr&){});
        WHEN("subscribe with lambda with specified type + error + on_completed")
            validate_subscribe([](const int&){}, [](const std::exception_ptr&){}, [](){});
        WHEN("subscribe with lambda with specified type + on_completed")
            validate_subscribe([](const int&){}, [](const std::exception_ptr&){}, [](){});
        WHEN("subscribe with generic lambda ")
            validate_subscribe([](const auto&){});
        WHEN("subscribe with specific_observer")
            validate_subscribe(rpp::specific_observer<int>{});
        WHEN("subscribe with dynamic_observer")
            validate_subscribe(rpp::dynamic_observer<int>{});
        WHEN("subscribe with specific_subscriber")
            validate_subscribe(rpp::specific_subscriber{rpp::specific_observer<int>{}});
        WHEN("subscribe with dynamic_subscriber")
            validate_subscribe(rpp::dynamic_subscriber<int>{});
        WHEN("subscribe with subscription + specific_observer")
            validate_subscribe(rpp::composite_subscription{}, rpp::specific_observer<int>{});
        WHEN("subscribe with subscription + dynamic_observer")
            validate_subscribe(rpp::composite_subscription{}, rpp::dynamic_observer<int>{});
        WHEN("subscribe with subscription + lambda")
            validate_subscribe(rpp::composite_subscription{}, [](const int&){});
    };

    GIVEN("specific_observable")
        validate_observable(rpp::specific_observable([&](const rpp::dynamic_subscriber<int>&) {++subscribe_count;}));

    GIVEN("dynamic_observable")
        validate_observable(rpp::dynamic_observable([&](const rpp::dynamic_subscriber<int>&) {++subscribe_count;}));
}

template<typename ObserverGetValue, bool is_move = false, bool is_const = false>
static void TestObserverTypes(const std::string then_description, int copy_count, int move_count)
{
    GIVEN("observer and observable of same type")
    {
        std::conditional_t<is_const, const copy_count_tracker, copy_count_tracker> tracker{};
        const auto observer             = rpp::dynamic_observer{[](ObserverGetValue) {  }};

        const auto observable = rpp::observable::create([&](const rpp::dynamic_subscriber<copy_count_tracker>& sub)
        {
            if constexpr (is_move)
                sub.on_next(std::move(tracker));
            else
                sub.on_next(tracker);
        });

        WHEN("subscribe called for observble")
        {
            observable.subscribe(observer);

            THEN(then_description)
            {
                CHECK(tracker.get_copy_count() == copy_count);
                CHECK(tracker.get_move_count() == move_count);
            }
        }
    }
}

SCENARIO("specific_observable doesn't produce extra copies for lambda", "[observable][track_copy]")
{
    GIVEN("observer and specific_observable of same type")
    {
        copy_count_tracker tracker{};
        const auto observer             = rpp::dynamic_observer{[](int) {  }};

        const auto observable = rpp::observable::create([tracker](const rpp::dynamic_subscriber<int>& sub)
        {
            sub.on_next(123);
        });

        WHEN("subscribe called for observble")
        {
            observable.subscribe(observer);

            THEN("One copy of tracker into lambda, one move of lambda into internal state")
            {
                CHECK(tracker.get_copy_count() == 1);
                CHECK(tracker.get_move_count() == 1);
            }
            AND_WHEN("Make copy of observable")
            {
                auto copy_of_observable = observable;
                THEN("One more copy of lambda")
                {
                    CHECK(tracker.get_copy_count() == 2);
                    CHECK(tracker.get_move_count() == 1);
                }
            }
        }

    }
}

SCENARIO("dynamic_observable doesn't produce extra copies for lambda", "[observable][track_copy]")
{
    GIVEN("observer and dynamic_observable of same type")
    {
        copy_count_tracker tracker{};
        const auto observer             = rpp::dynamic_observer{[](int) {  }};

        const auto observable = rpp::observable::create([tracker](const rpp::dynamic_subscriber<int>& sub)
        {
            sub.on_next(123);
        }).as_dynamic();

        WHEN("subscribe called for observble")
        {
            observable.subscribe(observer);

            THEN("One copy of tracker into lambda, one move of lambda into internal state and one move to dynamic observable")
            {
                CHECK(tracker.get_copy_count() == 1);
                CHECK(tracker.get_move_count() == 2);
            }
            AND_WHEN("Make copy of observable")
            {
                auto copy_of_observable = observable;
                THEN("No any new copies of lambda")
                {
                    CHECK(tracker.get_copy_count() == 1);
                    CHECK(tracker.get_move_count() == 2);
                }
            }
        }

    }
}

SCENARIO("Verify copy when observer take lvalue from lvalue&", "[observable][track_copy]")
{
    TestObserverTypes<copy_count_tracker>("1 copy to final lambda", 1, 0);
}

SCENARIO("Verify copy when observer take lvalue from move", "[observable][track_copy]")
{
    TestObserverTypes<copy_count_tracker, true>("1 move to final lambda", 0, 1);
}

SCENARIO("Verify copy when observer take lvalue from const lvalue&", "[observable][track_copy]")
{
    TestObserverTypes<copy_count_tracker,false, true>("1 copy to final lambda", 1, 0);
}

SCENARIO("Verify copy when observer take const lvalue& from lvalue&", "[observable][track_copy]")
{
    TestObserverTypes<const copy_count_tracker&>("no copies", 0, 0);
}

SCENARIO("Verify copy when observer take const lvalue& from move", "[observable][track_copy]")
{
    TestObserverTypes<const copy_count_tracker&, true>("no copies", 0, 0);
}

SCENARIO("Verify copy when observer take const lvalue& from const lvalue&", "[observable][track_copy]")
{
    TestObserverTypes<const copy_count_tracker&,false, true>("no copies", 0, 0);
}

SCENARIO("source::just")
{
    mock_observer<copy_count_tracker> mock{false};

    GIVEN("observable with copied item")
    {
        copy_count_tracker v{};
        auto obs = rpp::observable::just(v);
        WHEN("subscribe on this observable")
        {
            obs.subscribe(mock);
            THEN("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 1); // 1 copy into function for observable
                CHECK(v.get_move_count() == 1); // 1 move into observable
            }
        }
    }
    GIVEN("observable with moved item")
    {
        copy_count_tracker v{};
        auto obs = rpp::observable::just(std::move(v));
        WHEN("subscribe on this observable")
        {
            obs.subscribe(mock);
            THEN("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 0);
                CHECK(v.get_move_count() == 2); // 1 move into function for observable + 1 move into observable
            }
        }
    }
    GIVEN("observable with scheduler")
    {
        copy_count_tracker v{};
        auto obs = rpp::observable::just(v, rpp::schedulers::new_thread{});
        WHEN("subscribe on this observable")
        {
            THEN("value obtained")
            {
                std::promise<bool> prom{};
                auto fut = prom.get_future();
                obs.subscribe([&](const copy_count_tracker&) {prom.set_value(true); });

                REQUIRE(fut.get());

                CHECK(v.get_copy_count() == 2); // 1 copy into function for observable + 1 copy to scheduler lambda
                CHECK(v.get_move_count() == 3); // 1 move into observable + 1 move to queue + 1 move from queue
            }
        }
    }
}