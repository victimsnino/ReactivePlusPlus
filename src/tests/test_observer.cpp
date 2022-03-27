// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
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

#include <catch2/catch_test_macros.hpp>
#include <rpp/observers.h>
#include <rpp/observers/state_observer.h>
#include <rpp/subscribers/dynamic_subscriber.h>

SCENARIO("on_next, on_error and on_completed can be obtained")
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

SCENARIO("Any observer can be casted to dynamic_observer")
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

SCENARIO("State observer copy-count for state")
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
            observer.on_error(std::make_exception_ptr(std::exception{}));
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

SCENARIO("State proxy calls to subscriber")
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