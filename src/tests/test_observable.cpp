//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "copy_count_tracker.hpp"
#include "mock_observer.hpp"
#include "rpp/schedulers/new_thread_scheduler.hpp"

#include <catch2/catch_test_macros.hpp>

#include <rpp/sources.hpp>
#include <rpp/observables.hpp>
#include <rpp/observers.hpp>
#include <rpp/subscribers.hpp>
#include <rpp/subjects.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/operators/publish.hpp>

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

SCENARIO("Observable with exception", "[observable]")
{
    GIVEN("Observable with error")
    {
        auto obs = rpp::source::create<int>([](const auto&)
        {
                throw std::runtime_error{ "" };
        });
        WHEN("subscribe on it")
        {
            auto mock = mock_observer<int>();
            obs.subscribe(mock);
            THEN("exception provided")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
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

SCENARIO("base observables", "[observable]")
{
    mock_observer<int> mock{ };

    GIVEN("empty")
    {
        auto observable = rpp::observable::empty<int>();
        WHEN("subscribe on this observable")
        {
            observable.subscribe(mock);
            THEN("only on_completed called")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    GIVEN("never")
    {
        auto observable = rpp::observable::never<int>();
        WHEN("subscribe on this observable")
        {
            observable.subscribe(mock);
            THEN("no any callbacks")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
    GIVEN("error")
    {
        auto observable = rpp::observable::error<int>(std::make_exception_ptr(std::runtime_error{"MY EXCEPTION"}));
        WHEN("subscribe on this observable")
        {
            observable.subscribe(mock);
            THEN("only on_error callback once")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("blocking observable", "[observable]")
{
    GIVEN("observable with wait")
    {
        auto obs = rpp::source::create<int>([](const auto& sub)
        {
            std::this_thread::sleep_for(std::chrono::seconds{1});
            sub.on_completed();
        });
        WHEN("subscribe on it via as_blocking")
        {
            auto mock = mock_observer<int>();
            obs.as_blocking().subscribe(mock);
            THEN("obtain on_completed")
            {
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    GIVEN("observable with error")
    {
        auto obs = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));
        WHEN("subscribe on it via as_blocking")
        {
            auto mock = mock_observer<int>();
            obs.as_blocking().subscribe(mock);
            THEN("obtain on_error")
            {
                CHECK(mock.get_on_error_count() == 1);
            }
        }
    }
}

TEST_CASE("Observable size should be equal to size of state", "[observable]")
{
    SECTION("specific_observable")
    {
        auto action     = [](const auto&) {};
        auto observable = rpp::source::create<int>(action);
        CHECK(sizeof(observable) == sizeof(action));
    }

    SECTION("dynamic_observable")
    {
        auto observable = rpp::source::create<int>([](const auto&) {}).as_dynamic<>();
        CHECK(sizeof(observable) == sizeof(std::shared_ptr<int>));
    }
}
