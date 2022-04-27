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
        auto               obs = rpp::observable::just(v);
        WHEN("subscribe on this observable")
        {
            obs.subscribe(mock);
            THEN("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 1); // 1 copy into array
                CHECK(v.get_move_count() <= 2); // 1 move of array into lambda + 1 move lambda into observable
            }
        }
    }
    GIVEN("observable with moved item")
    {
        copy_count_tracker v{};
        auto               obs = rpp::observable::just(std::move(v));
        WHEN("subscribe on this observable")
        {
            obs.subscribe(mock);
            THEN("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 0);
                CHECK(v.get_move_count() <= 3); // 1 move into array + 1 move array to function for observable + 1 move into observable
            }
        }
    }
    GIVEN("observable with copied item + use_sahred")
    {
        copy_count_tracker v{};
        auto               obs = rpp::observable::just<rpp::memory_model::use_shared>(v);
        WHEN("subscribe on this observable")
        {
            obs.subscribe(mock);
            THEN("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 1); // 1 copy into shared_ptr
                CHECK(v.get_move_count() == 0);
            }
        }
    }
    GIVEN("observable with moved item + use_shared")
    {
        copy_count_tracker v{};
        auto               obs = rpp::observable::just<rpp::memory_model::use_shared>(std::move(v));
        WHEN("subscribe on this observable")
        {
            obs.subscribe(mock);
            THEN("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 0);
                CHECK(v.get_move_count() == 1); // 1 move into shared_ptr
            }
        }
    }
}

SCENARIO("just variadic")
{
    auto mock = mock_observer<int>();
    GIVEN("observable just variadic")
    {
        auto obs = rpp::source::just(1, 2, 3, 4, 5, 6);
        WHEN("subscribe on it")
        {
            obs.subscribe(mock);
            THEN("observer obtains values in the same order")
            {
                CHECK(mock.get_received_values() == std::vector{ 1, 2, 3, 4, 5, 6 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

SCENARIO("base observables")
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

SCENARIO("blocking observable")
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

SCENARIO("connectable observable")
{
    auto mock = mock_observer<int>{};
    GIVEN("source and connectable observable from it")
    {
        auto source = rpp::source::just(1);
        auto connectable = rpp::connectable_observable{ source, rpp::subjects::publish_subject<int>{} };
        WHEN("subscribe on connectable")
        {
            auto sub = connectable.subscribe(mock);
            THEN("nothing happens")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(sub.is_subscribed());
            }
            AND_WHEN("call connect")
            {
                auto sub_connectable = connectable.connect();
                THEN("observer obtains values")
                {
                    CHECK(mock.get_total_on_next_count() == 1);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                    CHECK(sub.is_subscribed() == false);
                    CHECK(sub_connectable.is_subscribed() == false);
                }
            }
        }
    }
    GIVEN("subject as source and connectable observable from it")
    {
        auto source = rpp::subjects::publish_subject<int>();
        auto connectable = rpp::connectable_observable{ source.get_observable(), rpp::subjects::publish_subject<int>{} };
        WHEN("subscribe on connectable and connect")
        {
            auto sub = connectable.subscribe(mock);
            auto sub_connectable = connectable.connect();

            AND_WHEN("unsubscribe connected subscription before any values from source")
            {
                sub_connectable.unsubscribe();
                source.get_subscriber().on_next(1);
                THEN("subscriber obtains nothing")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                    CHECK(sub.is_subscribed());
                    CHECK(!sub_connectable.is_subscribed());
                    CHECK(source.get_subscriber().is_subscribed());
                }
                AND_WHEN("connect again and send values")
                {
                    auto new_sub_connectable = connectable.connect();
                    source.get_subscriber().on_next(1);

                    THEN("subscriber obtains values")
                    {
                        CHECK(mock.get_total_on_next_count() == 1);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                        CHECK(sub.is_subscribed());
                        CHECK(new_sub_connectable.is_subscribed());
                        CHECK(source.get_subscriber().is_subscribed());
                    }
                }
            }
            AND_WHEN("obtain on_completed")
            {
                // hack but dont break subscription of this subject
                source.get_subscriber().get_observer().on_completed();
                THEN("subscribe obtains on_completed and unsubscribe initiated")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                    CHECK(!sub.is_subscribed());
                    CHECK(!sub_connectable.is_subscribed());
                }
                AND_WHEN("connect again and send values")
                {
                    auto new_sub_connectable = connectable.connect();
                    source.get_subscriber().on_next(1);
                    THEN("subscriber obtains nothing")
                    {
                        CHECK(mock.get_total_on_next_count() == 0);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                        CHECK(!sub.is_subscribed());
                        CHECK(!sub_connectable.is_subscribed());
                        CHECK(!new_sub_connectable.is_subscribed());
                    }
                }
            }
        }
    }
}