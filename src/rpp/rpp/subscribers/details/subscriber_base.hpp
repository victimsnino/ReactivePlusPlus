//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/subscriptions/composite_subscription.hpp>
#include <rpp/subscriptions/subscription_guard.hpp>

namespace rpp::details
{
/**
 * \brief base implementation of subscriber with possibility to obtain observer's callbacks, query subscription state, unsubscribe and etc. Each observer's callback checks for actual subscription
 * \tparam Type type of values expected by this subscriber
 */
class subscriber_base
{
public:
    subscriber_base(composite_subscription&& subscription = composite_subscription{})
        : m_subscription{ std::move(subscription) } { }

    subscriber_base(const composite_subscription& subscription)
        : m_subscription{subscription}{ }
    

    const composite_subscription& get_subscription() const
    {
        return m_subscription;
    }

    [[nodiscard]] bool is_subscribed() const
    {
        return m_subscription.is_subscribed();
    }

    void unsubscribe() const
    {
        m_subscription.unsubscribe();
    }

protected:
    void do_if_subscribed_and_unsubscribe(const auto& callable) const
    {
        if (!is_subscribed()) [[unlikely]]
            return;

        subscription_guard guard{m_subscription};
        callable();
    }

private:
    composite_subscription m_subscription{};
};
} // namespace rpp::details
