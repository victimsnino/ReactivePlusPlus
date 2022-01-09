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

#include <catch2/catch_test_macros.hpp>

#include <rpp/observable.h>
#include <rpp/observer.h>
#include <rpp/subscriber.h>

#include <exception>

SCENARIO("Subscriber unsubscribes when obtains on_error or on_completed", "[subscriber]")
{
    GIVEN("observer")
    {
        size_t on_next_called_count = 0;
        size_t on_error_called_count = 0;
        size_t on_completed_called_count = 0;
        const auto observer = rpp::observer{[&](int) { ++on_next_called_count; },
                                            [&](const std::exception_ptr&) { ++on_error_called_count; },
                                            [&]() { ++on_completed_called_count; }
        };

        WHEN("Subscribe on observable with first on_error")
        {
            rpp::observable{[](const rpp::subscriber<int>& sub)
            {
                sub.on_error(std::make_exception_ptr(std::exception{}));

                sub.on_next(1);
                sub.on_completed();
            }}.subscribe(observer);

            THEN("No calls after on_error")
            {
                CHECK(on_next_called_count == 0);
                CHECK(on_error_called_count == 1);
                CHECK(on_completed_called_count == 0);
            }
        }
    }
}

SCENARIO("Subscriber is not active after on_completed or unsubscribe", "[subscriber]")
{
    GIVEN("observer")
    {
        size_t on_next_called_count = 0;
        size_t on_error_called_count = 0;
        size_t on_completed_called_count = 0;
        const auto observer = rpp::observer{[&](int) { ++on_next_called_count; },
                                            [&](const std::exception_ptr&) { ++on_error_called_count; },
                                            [&]() { ++on_completed_called_count; }};

        auto subscriber = rpp::subscriber{observer};

        WHEN("Subscriber subscribes on observable with on_completed ")
        {
            const auto obs = rpp::observable{[](const rpp::subscriber<int>& sub) { sub.on_completed(); }};
            obs.subscribe(subscriber);

            THEN("Only one on_completed call first time")
            {
                CHECK(on_next_called_count == 0);
                CHECK(on_error_called_count == 0);
                CHECK(on_completed_called_count == 1);
            }
            AND_WHEN("The same subscriber subscribes second time")
            {
                obs.subscribe(subscriber);
                THEN("No any new calls happens")
                {
                    CHECK(on_next_called_count == 0);
                    CHECK(on_error_called_count == 0);
                    CHECK(on_completed_called_count == 1);
                }
            }
        }

        WHEN("Subscriber subscribes on observable with on_next")
        {
            const auto obs          = rpp::observable{[](const rpp::subscriber<int>& sub) { sub.on_next(1); }};
            auto       subscription = obs.subscribe(subscriber);

            THEN("Only one on_next call")
            {
                CHECK(on_next_called_count == 1);
                CHECK(on_error_called_count == 0);
                CHECK(on_completed_called_count == 0);
            }
            AND_WHEN("The same subscriber subscribes second time")
            {
                obs.subscribe(subscriber);
                THEN("One more on_next call")
                {
                    CHECK(on_next_called_count == 2);
                    CHECK(on_error_called_count == 0);
                    CHECK(on_completed_called_count == 0);
                }
            }
            AND_WHEN("The same subscriber unsubscribes and subscribes second time")
            {
                subscriber.unsubscribe();
                obs.subscribe(subscriber);
                THEN("No any new calls")
                {
                    CHECK(on_next_called_count == 1);
                    CHECK(on_error_called_count == 0);
                    CHECK(on_completed_called_count == 0);
                }
            }
            AND_WHEN("The subscription unsubscribes and subscriber subscribes second time")
            {
                subscription.unsubscribe();
                obs.subscribe(subscriber);
                THEN("No any new calls")
                {
                    CHECK(on_next_called_count == 1);
                    CHECK(on_error_called_count == 0);
                    CHECK(on_completed_called_count == 0);
                }
            }
        }
    }
}

SCENARIO("Subscriber obtains on_error when exception", "[subscriber]")
{
    GIVEN("observer and observable with exception")
    {
        size_t on_error_count = 0;
        size_t on_completed_count = 0;
        auto observer = rpp::observer{[](const double&) {},
                                      [&](const std::exception_ptr& ) {++on_error_count;},
                                      [&]() {++on_completed_count;} };
        auto observable = rpp::observable{[](const rpp::subscriber<double>& sub){throw std::exception("Test");}};

        WHEN("observer subscribes")
        {
            auto subscription = observable.subscribe(observer);
            THEN("on_error called once only")
            {
                CHECK(on_error_count == 1);
                CHECK(on_completed_count == 0);
                CHECK(subscription.is_subscribed() == false);
            }
        }
    }
}
