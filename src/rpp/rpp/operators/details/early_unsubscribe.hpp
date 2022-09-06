//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once
#include <rpp/subscriptions/composite_subscription.hpp>

#include <exception>

namespace rpp::details
{
struct early_unsubscribe_state
{
    early_unsubscribe_state(rpp::composite_subscription subscription_of_subscriber) : childs_subscriptions(subscription_of_subscriber.make_child()) {}

    // use this subscription as source for any chuld subscription that should be early unsubscribed
    rpp::composite_subscription childs_subscriptions{};
};
} // namespace rpp::details