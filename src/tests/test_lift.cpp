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

#include <catch2/catch_test_macros.hpp>

#include <rpp/observable.h>
#include <rpp/observer.h>
#include <rpp/subscriber.h>

SCENARIO("Observable can be lifted")
{
    GIVEN("Observable")
    {
        auto verifier   = copy_count_tracker{};
        auto observable = rpp::observable::create([verifier](const rpp::dynamic_subscriber<int>& sub)
        {
            sub.on_next(10);
            sub.on_next(5);
            sub.on_completed();
        });
        WHEN("Call lift")
        {
            int calls_internal = 0;
            auto new_observable = observable.lift([&](rpp::dynamic_subscriber<int> sub)
            {
                return rpp::specific_subscriber{sub.get_subscription(), [&, sub](int val)
                {
                    ++calls_internal;
                    sub.on_next(val);
                }};
            });

            AND_WHEN("subscribe unsubscribed subscriber")
            {
                int calls_external = 0;

                auto subscriber = rpp::specific_subscriber{[&](int){++calls_external;}};
                subscriber.unsubscribe();

                new_observable.subscribe(subscriber);

                THEN("No any calls obtained")
                {
                    CHECK(calls_internal == 0);
                    CHECK(calls_external == 0);
                }
            }
        }
        WHEN("Call lift as lvalue")
        {
            auto initial_copy_count = verifier.get_copy_count();
            auto initial_move_count = verifier.get_move_count();

            auto new_observable = observable.lift([](rpp::dynamic_subscriber<double> sub)
            {
                return rpp::dynamic_subscriber{sub.get_subscription(), [sub](int val)
                {
                    sub.on_next(static_cast<double>(val) / 2);
                }};
            });
            THEN("On subscribe obtain modified values")
            {
                std::vector<double> obtained_values{};
                new_observable.subscribe([&](double v) { obtained_values.push_back(v);});

                CHECK(obtained_values == std::vector{5, 2.5});
            }
            AND_THEN("One copy to lambda + one move to new observable")
            {
                CHECK(verifier.get_copy_count() == initial_copy_count +1);
                CHECK(verifier.get_move_count() == initial_move_count +1);
            }
        }
        WHEN("Call lift as rvalue")
        {
            auto initial_copy_count = verifier.get_copy_count();
            auto initial_move_count = verifier.get_move_count();

            auto new_observable = std::move(observable).lift([](rpp::dynamic_subscriber<double> sub)
            {
                return rpp::specific_subscriber{sub.get_subscription(), [sub](int val)
                {
                    sub.on_next(static_cast<double>(val) / 2);
                }};
            });
            THEN("On subscribe obtain modified values")
            {
                std::vector<double> obtained_values{};
                new_observable.subscribe([&](double v) { obtained_values.push_back(v);});

                CHECK(obtained_values == std::vector{5, 2.5});
            }
            AND_THEN("One move to lambda + one move to new observable")
            {
                CHECK(verifier.get_copy_count() == initial_copy_count);
                CHECK(verifier.get_move_count() == initial_move_count +2);
            }
        }
    }
}
