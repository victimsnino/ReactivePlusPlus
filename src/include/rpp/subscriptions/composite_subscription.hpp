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

#include <algorithm>
#include <mutex>
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

    composite_subscription(const composite_subscription&)     = default;
    composite_subscription(composite_subscription&&) noexcept = default;

    /**
     * \brief Add any other subscription to this as dependent
     */
    subscription_base add(const subscription_base& sub = subscription_base{}) const
    {
        if (&sub != this)
        {
            if (const auto pstate = get_state())
                static_cast<state*>(pstate)->add(sub);
        }
        return sub;
    }

    /**
     * \brief Add callback/function subscription to this as dependent
     */
    subscription_base add(const callback_subscription& sub) const
    {
        return add(static_cast<const subscription_base&>(sub));
    }

    composite_subscription make_child() const
    {
        composite_subscription ret{};
        add(ret);
        return ret;
    }

private:
    class state final : public details::subscription_state
    {
    public:
        state(std::vector<subscription_base>&& deps)
            : m_deps{std::move(deps)} {}

        void add(const subscription_base& sub)
        {
            while (true)
            {
                DepsState expected{DepsState::None};
                if (m_state.compare_exchange_strong(expected, DepsState::Add))
                {
                    m_deps.push_back(sub);

                    m_state.store(DepsState::None);
                    return;
                }

                if (expected == DepsState::Unsubscribed)
                {
                    sub.unsubscribe();
                    return;
                }
            }
        }

    private:
        void on_unsubscribe() override
        {
            while (true)
            {
                DepsState expected{DepsState::None};
                if (m_state.compare_exchange_strong(expected, DepsState::Unsubscribed))
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
            Add,         //< set it during adding new element into deps. After success -> FallBack to None
            Unsubscribed //< permanent state after unsubscribe
        };

        std::atomic<DepsState>         m_state{DepsState::None};
        std::vector<subscription_base> m_deps{};
    };
};
} // namespace rpp
