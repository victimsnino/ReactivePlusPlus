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

#include "mock_observer.h"

#include <catch2/catch_test_macros.hpp>

#include <rpp/observable.h>
#include <rpp/observer.h>
#include <rpp/subscriber.h>

SCENARIO("Any type of observer can be passed to any type of subscriber", "[subscriber]")
{
    auto validate_observer = [](auto observer)
    {
        WHEN("passed to specific subscriber")
        {
            auto sub = rpp::specific_subscriber{observer};
            THEN("Obtained subscriber of the same type")
            {
                using ObsType = std::remove_cv_t<decltype(observer)>;
                using ObsValueType = rpp::utils::extract_observer_type_t<ObsType>;

                static_assert(std::is_same_v<ObsValueType, rpp::utils::extract_subscriber_type_t<decltype(sub)>>);
                static_assert(std::is_same_v<rpp::specific_subscriber<ObsValueType, ObsType>, decltype(sub)>);
            }
        }
        WHEN("passed to dynamic subscriber")
        {
            auto sub = rpp::dynamic_subscriber{observer};
            THEN("Obtained subscriber of the same type")
            {
                using ObsType = std::remove_cv_t<decltype(observer)>;
                using ObsValueType = rpp::utils::extract_observer_type_t<ObsType>;

                static_assert(std::is_same_v<ObsValueType, rpp::utils::extract_subscriber_type_t<decltype(sub)>>);
                static_assert(std::is_same_v<rpp::dynamic_subscriber<ObsValueType>, decltype(sub)>);
            }
        }
    };

    GIVEN("dynamic observer")
        validate_observer(rpp::dynamic_observer{[](const int& ) {}});

    GIVEN("specific observer")
        validate_observer(rpp::specific_observer{[](const int&) {}});

    GIVEN("mock observer")
        validate_observer(mock_observer<int>{});
}

SCENARIO("Each type of call can be forwarded to observer via subscriber", "[subscriber]")
{
    GIVEN("observer")
    {
        auto observer = mock_observer<int>{};

        auto validate_subscriber = [&](auto sub)
        {
            AND_WHEN("Called on_next")
            {
                sub.on_next(1);
                THEN("Observer obtain on_next")
                {
                    CHECK(observer.get_total_on_next_count() == 1);
                    CHECK(observer.get_on_error_count() == 0);
                    CHECK(observer.get_on_completed_count() == 0);
                }
            }
            AND_WHEN("Called on_error")
            {
                sub.on_error(std::make_exception_ptr(std::exception{}));
                THEN("Observer obtain on_error")
                {
                    CHECK(observer.get_total_on_next_count() == 0);
                    CHECK(observer.get_on_error_count() == 1);
                    CHECK(observer.get_on_completed_count() == 0);
                }
            }
            AND_WHEN("Called on_completed")
            {
                sub.on_completed();
                THEN("Observer obtain on_completed")
                {
                    CHECK(observer.get_total_on_next_count() == 0);
                    CHECK(observer.get_on_error_count() == 0);
                    CHECK(observer.get_on_completed_count() == 1);
                }
            }
        };

        WHEN("passed to specific subscriber")
            validate_subscriber(rpp::specific_subscriber{observer});
        WHEN("passed to dynamic subscriber")
            validate_subscriber(rpp::dynamic_subscriber{observer});
    }
}

SCENARIO("Subscriber reacts on on_error or on_completed", "[subscriber]")
{
    const auto observer = mock_observer<int>();

    auto validate = [&](auto sub)
    {
        WHEN("subscriber calls first on_error and some calls after")
        {
            sub.on_error(std::make_exception_ptr(std::exception{}));

            sub.on_next(1);
            sub.on_completed();

            THEN("observer doesn't obtain calls after on_error")
            {
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 1);
                CHECK(observer.get_on_completed_count() == 0);
            }
        }
    };

    GIVEN("dynamic_subscriber")
        validate(rpp::dynamic_subscriber{observer});

    GIVEN("specific_subscriber")
        validate(rpp::specific_subscriber{observer});
}

SCENARIO("Subscriber is not active after on_completed or unsubscribe", "[subscriber]")
{
    const auto observer = mock_observer<int>();

    auto validate = [&](auto subscriber)
    {
        WHEN("Subscriber subscribes on observable with on_completed ")
        {
            const auto obs = rpp::observable::create<int>([](const auto& sub)
            {
                sub.on_completed();
            });
            obs.subscribe(subscriber);

            THEN("Only one on_completed call first time")
            {
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
            AND_WHEN("The same subscriber subscribes second time")
            {
                obs.subscribe(subscriber);
                THEN("No any new calls happens")
                {
                    CHECK(observer.get_total_on_next_count() == 0);
                    CHECK(observer.get_on_error_count() == 0);
                    CHECK(observer.get_on_completed_count() == 1);
                }
            }
        }

        WHEN("Subscriber subscribes on observable with on_next")
        {
            const auto obs = rpp::observable::create<int>([](const auto& sub) { sub.on_next(1); });
            auto       subscription = obs.subscribe(subscriber);

            THEN("Only one on_next call")
            {
                CHECK(observer.get_total_on_next_count() == 1);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 0);
            }
            AND_WHEN("The same subscriber subscribes second time")
            {
                obs.subscribe(subscriber);
                THEN("One more on_next call")
                {
                    CHECK(observer.get_total_on_next_count() == 2);
                    CHECK(observer.get_on_error_count() == 0);
                    CHECK(observer.get_on_completed_count() == 0);
                }
            }
            AND_WHEN("The same subscriber unsubscribes and subscribes second time")
            {
                subscriber.unsubscribe();
                obs.subscribe(subscriber);
                THEN("No any new calls")
                {
                    CHECK(observer.get_total_on_next_count() == 1);
                    CHECK(observer.get_on_error_count() == 0);
                    CHECK(observer.get_on_completed_count() == 0);
                }
            }
            AND_WHEN("The subscription unsubscribes and subscriber subscribes second time")
            {
                subscription.unsubscribe();
                obs.subscribe(subscriber);
                THEN("No any new calls")
                {
                    CHECK(observer.get_total_on_next_count() == 1);
                    CHECK(observer.get_on_error_count() == 0);
                    CHECK(observer.get_on_completed_count() == 0);
                }
            }
        }
    };

    GIVEN("dynamic_subscriber")
        validate(rpp::dynamic_subscriber{observer});

    GIVEN("specific_subscriber")
        validate(rpp::specific_subscriber{observer});
}

SCENARIO("Subscriber obtains on_error when exception", "[subscriber]")
{
    GIVEN("observer and observable with exception")
    {
        auto observable = rpp::observable::create<int>([](const auto&)
        {
            throw std::runtime_error("Test");
        });

        auto observer = mock_observer<int>{};

        auto validate = [&](auto sub)
        {
            WHEN("observer subscribes")
            {
                auto subscription = observable.subscribe(sub);
                THEN("on_error called once only")
                {
                    CHECK(observer.get_total_on_next_count() == 0);
                    CHECK(observer.get_on_error_count() == 1);
                    CHECK(observer.get_on_completed_count() == 0);
                    CHECK(subscription.is_subscribed() == false);
                }
            }
        };

        AND_GIVEN("dynamic_subscriber")
            validate(rpp::dynamic_subscriber{observer});

        AND_GIVEN("specific_subscriber")
            validate(rpp::specific_subscriber{observer});
    }
}

SCENARIO("Subscriber can be generated by lambdas", "[subscriber]")
{
    {
        auto temp_1 = rpp::dynamic_subscriber{[](int       ) {}};
        auto temp_2 = rpp::dynamic_subscriber{[](const int&) {}, [](std::exception_ptr) {}};
        auto temp_3 = rpp::dynamic_subscriber{[](const int&) {}, [](std::exception_ptr) {}, []() {}};
        auto temp_4 = rpp::dynamic_subscriber{[](const int&) {}, []() {}};
    }

    {
        auto temp_1 = rpp::specific_subscriber{[](int       ) {}};
        auto temp_2 = rpp::specific_subscriber{[](const int&) {}, [](std::exception_ptr) {}};
        auto temp_3 = rpp::specific_subscriber{[](const int&) {}, [](std::exception_ptr) {}, []() {}};
        auto temp_4 = rpp::specific_subscriber{[](const int&) {}, []() {}};
    }


    auto on_next      = [](const int&               ) {};
    auto on_error     = [](const std::exception_ptr&) {};
    auto on_completed = []() {};

    static_assert(rpp::utils::is_observer_constructible_v<int, decltype(on_next)> == true);
    static_assert(rpp::utils::is_observer_constructible_v<int, decltype(on_next), decltype(on_error)> == true);
    static_assert(rpp::utils::is_observer_constructible_v<int, decltype(on_next), decltype(on_error), decltype(on_completed)> == true);
}
