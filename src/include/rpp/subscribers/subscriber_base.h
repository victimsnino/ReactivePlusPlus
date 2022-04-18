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

#include <rpp/observers/interface_observer.h>
#include <rpp/subscriptions/composite_subscription.h>
#include <rpp/subscriptions/subscription_guard.h>

namespace rpp::details
{
struct subscriber_tag {};

/**
 * \brief base implementation of subscriber with possibility to obtain observer's callbacks, query subscription state, unsubscribe and etc. Each observer's callback checks for actual subscription
 * \tparam Type type of values expected by this subscriber
 */
template<constraint::decayed_type Type>
class subscriber_base
        : public interface_observer<Type>
        , public subscriber_tag
{
public:
    subscriber_base(composite_subscription&& subscription = composite_subscription{})
        : m_subscription{ std::move(subscription) } { }

    subscriber_base(const composite_subscription& subscription)
        : m_subscription{subscription}{ }

    subscriber_base(const subscriber_base&)     = default;
    subscriber_base(subscriber_base&&) noexcept = default;

    void on_next(const Type& val) const final
    {
        if (!is_subscribed())
            return;

        try
        {
            on_next_impl(val);
        }
        catch (const std::exception& err)
        {
            on_error(std::make_exception_ptr(err));
        }
    }

    void on_next(Type&& val) const final
    {
        if (!is_subscribed())
            return;

        try
        {
            on_next_impl(std::move(val));
        }
        catch (const std::exception& err)
        {
            on_error(std::make_exception_ptr(err));
        }
    }

    void on_error(const std::exception_ptr& err) const final
    {
        if (!is_subscribed())
            return;

        subscription_guard guard{m_subscription};
        on_error_impl(err);
    }

    void on_completed() const final
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
