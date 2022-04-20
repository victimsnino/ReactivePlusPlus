//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <rpp/subscriptions/subscription_base.hpp>
#include <rpp/subscriptions/callback_subscription.hpp>
#include <rpp/subscriptions/composite_subscription.hpp>

#include <catch2/catch_test_macros..hpp>

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
