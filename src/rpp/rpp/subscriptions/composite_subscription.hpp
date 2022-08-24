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

#include <rpp/subscriptions/subscription_base.hpp>
#include <rpp/subscriptions/callback_subscription.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/subscriptions/constraints.hpp>

#include <algorithm>
#include <vector>

namespace rpp
{
/**
 * \brief rpp::subscription_base with ability to add some dependent subscriptions as a part of this one: in case of initiation of unsubscribe of this subscription, then any dependent subscriptions will be unsubscribed too
 */
class composite_subscription final : public subscription_base
{
public:
    template<std::convertible_to<subscription_base> ...Subs>
    explicit composite_subscription(const Subs&...subs) requires (!rpp::constraint::variadic_is_same_type< composite_subscription>)
        : subscription_base{std::make_shared<state>(std::vector<subscription_base>{subs...})} {}

    composite_subscription(const composite_subscription&)                      = default;
    composite_subscription(composite_subscription&&) noexcept                  = default;
    composite_subscription& operator=(const composite_subscription& other)     = default;
    composite_subscription& operator=(composite_subscription&& other) noexcept = default;

    /**
     * \brief Add any other subscription to this as dependent
     */
    template<constraint::subscription TSub = subscription_base>
    TSub add(const TSub& sub = TSub{}) const
    {
        if (static_cast<const subscription_base*>(&sub) == static_cast<const subscription_base*>(this))
            return sub;

        if (const auto pstate = get_state())
            static_cast<state*>(pstate)->add(sub);
        else
            sub.unsubscribe();
        return sub;
    }

    /**
     * \brief Add callback/function subscription to this as dependent
     */
    callback_subscription add(const callback_subscription& sub) const
    {
        return add<callback_subscription>(sub);
    }

    composite_subscription make_child() const
    {
        composite_subscription ret{};
        add(ret);
        ret.add([ret, state = std::weak_ptr{ std::static_pointer_cast<state>(get_state_as_shared()) }]
            {
                // add cleanup
                if (const auto shared = state.lock())
                    shared->remove(ret);
            });
        return ret;
    }

    void remove(const subscription_base& sub) const
    {
        if (const auto pstate = get_state())
            static_cast<state*>(pstate)->remove(sub);
    }

    bool is_empty() const
    {
        return !get_state();
    }

    static composite_subscription empty()
    {
        return composite_subscription{empty_tag{}};
    }

private:
    struct empty_tag{};

    composite_subscription(const empty_tag&)
        : subscription_base{std::shared_ptr<details::subscription_state>{}} {}

    class state final : public details::subscription_state
    {
    public:
        state(std::vector<subscription_base>&& deps)
            : m_deps{std::move(deps)} {}

        void add(const subscription_base& sub)
        {
            if (!sub.is_subscribed())
                return;

            while (true)
            {
                DepsState expected{DepsState::None};
                if (m_state.compare_exchange_strong(expected, DepsState::Edit, std::memory_order::acq_rel))
                {
                    m_deps.push_back(sub);
                    
                    m_state.store(DepsState::None, std::memory_order::release);
                    return;
                }

                if (expected == DepsState::Unsubscribed)
                {
                    sub.unsubscribe();
                    return;
                }
            }
        }

        void remove(const subscription_base& sub)
        {
            while (true)
            {
                DepsState expected{DepsState::None};
                if (m_state.compare_exchange_strong(expected, DepsState::Edit, std::memory_order::acq_rel))
                {
                    std::erase(m_deps, sub);

                    m_state.store(DepsState::None, std::memory_order::release);
                    return;
                }

                if (expected == DepsState::Unsubscribed)
                    return;
            }
        }

    private:
        void on_unsubscribe() override
        {
            while (true)
            {
                DepsState expected{DepsState::None};
                if (m_state.compare_exchange_strong(expected, DepsState::Unsubscribed, std::memory_order::acq_rel))
                {
                    std::ranges::for_each(m_deps, &subscription_base::unsubscribe);
                    m_deps.clear();
                    return;
                }
            }
        }

    private:
        enum class DepsState : uint8_t
        {
            None,        //< default state
            Edit,        //< set it during adding new element into deps or removing. After success -> FallBack to None
            Unsubscribed //< permanent state after unsubscribe
        };

        std::atomic<DepsState>         m_state{DepsState::None};
        std::vector<subscription_base> m_deps{};
    };
};
} // namespace rpp
