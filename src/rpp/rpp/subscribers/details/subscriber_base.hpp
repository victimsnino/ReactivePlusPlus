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

#include <rpp/observers/fwd.hpp>
#include <rpp/subscriptions/composite_subscription.hpp>
#include <rpp/subscriptions/subscription_guard.hpp>

namespace rpp::details
{
struct subscriber_tag {};

/**
 * \brief base implementation of subscriber with possibility to obtain observer's callbacks, query subscription state, unsubscribe and etc. Each observer's callback checks for actual subscription
 * \tparam Type type of values expected by this subscriber
 */
template<constraint::decayed_type Type>
class subscriber_base : public subscriber_tag
{
public:
    subscriber_base(composite_subscription&& subscription = composite_subscription{})
        : m_subscription{ std::move(subscription) } { }

    subscriber_base(const composite_subscription& subscription)
        : m_subscription{subscription}{ }

    subscriber_base(const subscriber_base&)     = default;
    subscriber_base(subscriber_base&&) noexcept = default;

    void on_next(constraint::decayed_same_as<Type> auto&& val) const
    {
        if (!is_subscribed())
            return;

        try
        {
            on_next_impl(std::forward<decltype(val)>(val));
        }
        catch (...)
        {
            on_error(std::current_exception());
        }
    }

    void on_error(const std::exception_ptr& err) const
    {
        if (!is_subscribed())
            return;

        subscription_guard guard{m_subscription};
        on_error_impl(err);
    }

    void on_completed() const
    {
        if (!is_subscribed())
            return;

        subscription_guard guard{m_subscription};
        on_completed_impl();
    }

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
    virtual void on_next_impl(const Type& v) const = 0;
    virtual void on_next_impl(Type&& v) const = 0;
    virtual void on_error_impl(const std::exception_ptr& err) const = 0;
    virtual void on_completed_impl() const = 0;

private:
    composite_subscription m_subscription{};
};
} // namespace rpp::details
