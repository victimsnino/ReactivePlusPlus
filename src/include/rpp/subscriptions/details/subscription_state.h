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

#include <atomic>

namespace rpp::details
{
/**
 * \brief Base implementation of subscription state used under-hood for rpp::subscription_base and its childs
 * \details subscription_state uses atomic_bool to track current state of the subscription and where unsubscribe should be called or not. Used as base implementation for more complicated states
 */
class subscription_state
{
public:
    subscription_state()          = default;
    virtual ~subscription_state() = default;

    subscription_state(const subscription_state&)     = delete;
    subscription_state(subscription_state&&) noexcept = delete;

    [[nodiscard]] bool is_subscribed() const
    {
        return m_is_subscribed.load();
    }

    void unsubscribe()
    {
        if (m_is_subscribed.exchange(false))
            on_unsubscribe();
    }

protected:
    /**
     * \brief Derrived action on unsubscribe. Will be called only ONCE!
     */
    virtual void on_unsubscribe() {}

private:
    std::atomic_bool m_is_subscribed{true};
};
} // namespace rpp::details
