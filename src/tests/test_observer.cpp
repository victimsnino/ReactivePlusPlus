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

#include <catch2/catch_test_macros.hpp>
#include <rpp/observers.hpp>
#include <rpp/observers/state_observer.hpp>
#include <rpp/subscribers/dynamic_subscriber.hpp>

SCENARIO("on_next, on_error and on_completed can be obtained", "[observer]")
{
    size_t on_next_count{}, on_error_count{}, on_completed_count{};

    auto validate_observer = [&](const auto& observer)
    {
        WHEN("Call on_next")
        {
            observer.on_next(1);
            THEN("on_next obtained")
            {
                REQUIRE(on_next_count == 1);
                REQUIRE(on_error_count == 0);
                REQUIRE(on_completed_count == 0);
            }
        }
        WHEN("Call on_error")
        {
            observer.on_error(std::make_exception_ptr(std::exception{}));
            THEN("on_next obtained")
            {
                REQUIRE(on_next_count == 0);
                REQUIRE(on_error_count == 1);
                REQUIRE(on_completed_count == 0);
            }
        }
        WHEN("Call on_completed")
        {
            observer.on_completed();
            THEN("on_next obtained")
            {
                REQUIRE(on_next_count == 0);
                REQUIRE(on_error_count == 0);
                REQUIRE(on_completed_count == 1);
            }
        }
    };
    GIVEN("specific_observer")
    {
        validate_observer(rpp::specific_observer{[&](const int&               ) { ++on_next_count; },
                                                 [&](const std::exception_ptr&) { ++on_error_count; },
                                                 [&]() { ++on_completed_count; }});
    }
    GIVEN("dynamic_observer")
    {
        validate_observer(rpp::dynamic_observer{[&](const int&               ) { ++on_next_count; },
                                                [&](const std::exception_ptr&) { ++on_error_count; },
                                                [&]() { ++on_completed_count; }});
    }
    GIVEN("dynamic_observer from specific_observer")
    {
        validate_observer(rpp::specific_observer{[&](const int&               ) { ++on_next_count; },
                                                 [&](const std::exception_ptr&) { ++on_error_count; },
                                                 [&]() { ++on_completed_count; }}.as_dynamic());
    }
}

SCENARIO("observer by default rethrow exceptions", "[observer]")
{
    GIVEN("observer with only on_next callback")
    {
        auto obs = rpp::make_specific_observer<int>([](int) {});
        WHEN("send on_error to it")
        {
            THEN("it rethrow error")
            {
                struct custom_error : std::runtime_error { using std::runtime_error::runtime_error; };
                auto err = custom_error{ "" };
                CHECK_THROWS_AS(obs.on_error(std::make_exception_ptr(err)), custom_error);
            }
        }
    }
}

SCENARIO("Any observer can be casted to dynamic_observer", "[observer]")
{
    auto validate_observer =[](const auto& observer)
    {
        WHEN("Call as_dynamic function")
        {
            auto dynamic_observer = observer.as_dynamic();

            THEN("Obtain dynamic_observer of same type")
            {
                static_assert(std::is_same<decltype(dynamic_observer), rpp::dynamic_observer<int>>{}, "Type of dynamic observer should be same!");
            }
        }

        WHEN("Construct dynamic_observer by constructor")
        {
            auto dynamic_observer = rpp::dynamic_observer{observer};

            THEN("Obtain dynamic_observer of same type")
            {
                static_assert(std::is_same<decltype(dynamic_observer), rpp::dynamic_observer<int>>{}, "Type of dynamic observer should be same!");
            }
        }
    };

    GIVEN("specific_observer")
    {
        validate_observer(rpp::specific_observer([](const int&) {}));
    }

    GIVEN("dynamic_observer")
    {
        validate_observer(rpp::dynamic_observer([](const int&) {}));
    }

    GIVEN("mock_observer")
    {
        validate_observer(mock_observer<int>{});
    }
}

SCENARIO("State observer copy-count for state", "[observer]")
{
    GIVEN("state")
    {
        auto state = copy_count_tracker{};
        auto make_observer = [](auto&& state)
        {
              auto observer = rpp::details::state_observer{std::forward<decltype(state)>(state),
                                                         [](int, const copy_count_tracker&) {},
                                                         [](const std::exception_ptr&, const copy_count_tracker&) {},
                                                         [](const copy_count_tracker&) {}};
            static_assert(std::is_same_v<rpp::utils::extract_observer_type_t<decltype(observer)>, int>);

            observer.on_next(1);
            observer.on_completed();
        };

        WHEN("pass it to state_observer")
        {
            make_observer(state);

            THEN("no extra copies")
            {
                CHECK(state.get_copy_count() == 1);
                CHECK(state.get_move_count() == 0);
            }
        }
        WHEN("move it to state_observer")
        {
            make_observer(std::move(state));

            THEN("no extra copies")
            {
                CHECK(state.get_copy_count() == 0);
                CHECK(state.get_move_count() == 1);
            }
        }
    }
}

SCENARIO("State proxy calls to subscriber", "[observer]")
{
    auto observer = mock_observer<int>{};
    auto subscriber = rpp::dynamic_subscriber{ observer };

    GIVEN("state_observer")
    {
        auto state_observer = rpp::details::state_observer{subscriber,
                                                           [](int v, rpp::dynamic_subscriber<int> sub)
                                                           {
                                                               sub.on_next(v);
                                                           },
                                                           rpp::details::forwarding_on_error{},
                                                           rpp::details::forwarding_on_completed{}};
        WHEN("call on_next")
        {
            state_observer.on_next(1);
            THEN("original observer obtains on_next")
                CHECK(observer.get_total_on_next_count() == 1);
        }
        WHEN("call on_error")
        {
            state_observer.on_error(std::exception_ptr{});
            THEN("original observer obtains on_error")
                CHECK(observer.get_on_error_count() == 1);
        }
        WHEN("call on_completed")
        {
            state_observer.on_completed();
            THEN("original observer obtains on_error")
                CHECK(observer.get_on_completed_count() == 1);
        }
    }
}