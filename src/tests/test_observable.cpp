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

#include <catch2/catch_test_macros.hpp>

#include <rpp/observer.h>
#include <rpp/subscriber.h>
#include <rpp/observables/observable.h>

#include <array>

SCENARIO("Observable should be subscribable")
{
    GIVEN("observer and observable of same type")
    {
        size_t     on_next_called_count = 0;
        const auto observer             = rpp::observer{[&](int) { ++on_next_called_count; }};

        size_t     on_subscribe_called_count = 0;
        const auto observable                = rpp::observable{[&](const rpp::subscriber<int>& sub)
        {
            ++on_subscribe_called_count;
            sub.on_next(123);
        }};

        WHEN("subscribe called for observble")
        {
            observable.subscribe(observer);

            THEN("OnSubscribe lambda called once")
            {
                CHECK(on_subscribe_called_count == 1);
            }
            AND_THEN("on_next lambda called once")
            {
                CHECK(on_next_called_count == 1);
            }
        }
    }

    GIVEN("observable with subscribier by non ref")
    {
        size_t     on_next_called_count = 0;
        const auto observer             = rpp::observer{[&](int                   ) { ++on_next_called_count; }};
        const auto observable           = rpp::observable{[](rpp::subscriber<int> sub) {sub.on_next(1);}};

        WHEN("subscribe called for observble")
        {
            observable.subscribe(observer);

            THEN("on_next lambda called once")
            {
                CHECK(on_next_called_count == 1);
            }
        }
    }
}

SCENARIO("on_next, on_error and on_completed can be called and obtained")
{
    GIVEN("ready observer")
    {
        size_t on_next_called_count = 0;
        size_t on_error_called_count = 0;
        size_t on_completed_called_count = 0;
        const auto observer = rpp::observer{[&](int) { ++on_next_called_count; },
                                            [&](std::exception_ptr) { ++on_error_called_count; },
                                            [&]() { ++on_completed_called_count; }
        };

        WHEN("subscribe on observable with on_next")
        {
            rpp::observable{[](const rpp::subscriber<int>& sub){sub.on_next(1);}}.subscribe(observer);

            THEN("on_next received once")
            {
                CHECK(on_next_called_count == 1);
                CHECK(on_error_called_count == 0);
                CHECK(on_completed_called_count == 0);
            }
        }
        WHEN("subscribe on observable with on_error")
        {
            rpp::observable{[](const rpp::subscriber<int>& sub){sub.on_error(std::make_exception_ptr(std::exception{}));}}.subscribe(observer);

            THEN("on_next received once")
            {
                CHECK(on_next_called_count == 0);
                CHECK(on_error_called_count == 1);
                CHECK(on_completed_called_count == 0);
            }
        }
        WHEN("subscribe on observable with on_completed")
        {
            rpp::observable{[](const rpp::subscriber<int>& sub){sub.on_completed();}}.subscribe(observer);

            THEN("on_next received once")
            {
                CHECK(on_next_called_count == 0);
                CHECK(on_error_called_count == 0);
                CHECK(on_completed_called_count == 1);
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
        const auto observer             = rpp::observer{[](ObserverGetValue) {  }};

        const auto observable = rpp::observable{[&](const rpp::subscriber<copy_count_tracker>& sub)
        {
            if constexpr (is_move)
                sub.on_next(std::move(tracker));
            else
                sub.on_next(tracker);
        }};

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

SCENARIO("observable doesn't produce extra copies for lambda", "[track_copy]")
{
    GIVEN("observer and observable of same type")
    {
        copy_count_tracker tracker{};
        const auto observer             = rpp::observer{[](int) {  }};

        const auto observable = rpp::observable{[tracker](const rpp::subscriber<int>& sub)
        {
            sub.on_next(123);
        }};

        WHEN("subscribe called for observble")
        {
            observable.subscribe(observer);

            THEN("One copy into lambda, one move of lambda into internal state")
            {
                CHECK(tracker.get_copy_count() == 1);
                CHECK(tracker.get_move_count() == 1);
            }
        }
    }
}

SCENARIO("Verify copy when observer take lvalue from lvalue&", "[track_copy]")
{
    TestObserverTypes<copy_count_tracker>("1 copy to final lambda", 1, 0);
}

SCENARIO("Verify copy when observer take const lvalue& from lvalue&", "[track_copy]")
{
    TestObserverTypes<const copy_count_tracker&>("no copies", 0, 0);
}

SCENARIO("Verify copy when observer take rvalue&& from lvalue&", "[track_copy]")
{
    TestObserverTypes<copy_count_tracker&&>("one copy to convert reference to temp", 1, 0);
}

SCENARIO("Verify copy when observer take lvalue& from lvalue&", "[track_copy]")
{
    TestObserverTypes<copy_count_tracker&>("no copies", 0, 0);
}

///

SCENARIO("Verify copy when observer take lvalue from move", "[track_copy]")
{
    TestObserverTypes<copy_count_tracker, true>("1 move to final lambda", 0, 1);
}

SCENARIO("Verify copy when observer take const lvalue& from move", "[track_copy]")
{
    TestObserverTypes<const copy_count_tracker&, true>("no copies", 0, 0);
}

SCENARIO("Verify copy when observer take rvalue&& from move", "[track_copy]")
{
    TestObserverTypes<copy_count_tracker&&, true>("no copies", 0, 0);
}

SCENARIO("Verify copy when observer take lvalue& from move", "[track_copy]")
{
    TestObserverTypes<copy_count_tracker&, true>("no copies", 0, 0);
}

///

SCENARIO("Verify copy when observer take lvalue from const lvalue&", "[track_copy]")
{
    TestObserverTypes<copy_count_tracker,false, true>("1 copy to final lambda", 1, 0);
}

SCENARIO("Verify copy when observer take const lvalue& from const lvalue&", "[track_copy]")
{
    TestObserverTypes<const copy_count_tracker&,false, true>("no copies", 0, 0);
}

SCENARIO("Verify copy when observer take rvalue&& from const lvalue&", "[track_copy]")
{
    TestObserverTypes<copy_count_tracker&&, false, true>("one copy to convert reference to temp", 1, 0);
}

SCENARIO("Verify copy when observer take lvalue& from const lvalue&", "[track_copy]")
{
    TestObserverTypes<copy_count_tracker&, false, true>("1 copy from const to temp", 1, 0);
}
