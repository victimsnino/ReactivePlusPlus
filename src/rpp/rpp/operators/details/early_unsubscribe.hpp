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
    early_unsubscribe_state(const composite_subscription& subscription_of_subscriber)
        : children_subscriptions(subscription_of_subscriber.make_child()) {}

    // use this subscription as source for any child subscription that should be early unsubscribed
    composite_subscription children_subscriptions;
};

struct early_unsubscribe_on_error
{
    void operator()(const std::exception_ptr&                       err,
                    const constraint::subscriber auto&              sub,
                    const std::shared_ptr<early_unsubscribe_state>& state) const
    {
        state->children_subscriptions.unsubscribe();
        sub.on_error(err);
    }
};
} // namespace rpp::details
