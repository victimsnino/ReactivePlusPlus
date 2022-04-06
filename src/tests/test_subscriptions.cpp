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

#include <rpp/subscriptions/subscription_base.h>
#include <rpp/subscriptions/callback_subscription.h>
#include <rpp/subscriptions/composite_subscription.h>

#include <catch2/catch_test_macros.hpp>

SCENARIO("subscriptions works as expected")
{
    GIVEN("subscription")
    {
        rpp::subscription_base sub{};
        REQUIRE(sub.is_subscribed());
        WHEN("call unsubscribe")
        {
            sub.unsubscribe();
            THEN("state changes")
            {
                REQUIRE(sub.is_subscribed() == false);
            }
        }
    }

    GIVEN("callback subscription")
    {
        size_t                     call_count{};
        rpp::callback_subscription sub{[&]() { ++call_count; }};
        REQUIRE(sub.is_subscribed());

        WHEN("call unsubscribe multiple times")
        {
            for (size_t i = 0; i < 10; ++i)
                sub.unsubscribe();

            THEN("state changes and callback called once")
            {
                REQUIRE(sub.is_subscribed() == false);
                REQUIRE(call_count == 1);
            }
        }
    }

    GIVEN("composite subscription")
    {
        rpp::subscription_base     sub{};
        size_t                     call_count{};
        rpp::callback_subscription callback_subscription{[&]() { ++call_count; }};

        rpp::composite_subscription composite{sub, callback_subscription};

        REQUIRE(composite.is_subscribed());

        WHEN("call unsubscribe multiple times")
        {
            composite.unsubscribe();
            composite.unsubscribe();
            composite.unsubscribe();

            THEN("state changes for all dependents subscriptions too")
            {
                REQUIRE(composite.is_subscribed() == false);
                REQUIRE(sub.is_subscribed() == false);
                REQUIRE(callback_subscription.is_subscribed() == false);
                REQUIRE(call_count == 1);
            }
        }
        WHEN("add new subscription and unsubscribe")
        {
            rpp::subscription_base new_sub{};
            composite.add(new_sub);
            composite.unsubscribe();

            THEN("state changes for all dependents subscriptions too")
            {
                REQUIRE(new_sub.is_subscribed() == false);
            }
        }
        WHEN("unsubscribe and add new subscription")
        {
            composite.unsubscribe();

            rpp::subscription_base new_sub{};
            composite.add(new_sub);

            THEN("new sub unsubscribed immediately")
            {
                REQUIRE(new_sub.is_subscribed() == false);
            }
        }
    }
}
