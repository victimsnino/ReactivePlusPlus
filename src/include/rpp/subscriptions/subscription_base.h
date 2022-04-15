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

#include <rpp/subscriptions/details/subscription_state.h>

#include <atomic>
#include <memory>

namespace rpp
{
/**
 * \brief Base subscription implementation used as base class/interface and core implementation for derrived subscriptions
 */
class subscription_base
{
protected:
    subscription_base(std::shared_ptr<details::subscription_state> state)
        : m_state{std::move(state)} {}

    details::subscription_state* get_state() const { return m_state.get(); }
public:
    subscription_base()
        : m_state{std::make_shared<details::subscription_state>()} {}

    virtual ~subscription_base() = default;

    static subscription_base empty() { return subscription_base{ nullptr }; }

    /**
     * \brief indicates current status of subscription
     */
    [[nodiscard]] bool is_subscribed() const
    {
        return m_state && m_state->is_subscribed();
    }

    /**
     * \brief initiates unsubscription process (if subscribed)
     */
    void unsubscribe() const
    {
        if (m_state)
            m_state->unsubscribe();
    }

private:
    std::shared_ptr<details::subscription_state> m_state{};
};

} // namespace rpp
