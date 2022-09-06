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
    composite_subscription(const Subs&...subs) requires (!rpp::constraint::variadic_is_same_type< composite_subscription>)
        : subscription_base{std::make_shared<state>(std::vector<std::shared_ptr<details::subscription_state>>{subs.get_state()...})} {}

    composite_subscription(const composite_subscription&)                      = default;
    composite_subscription(composite_subscription&&) noexcept                  = default;
    composite_subscription& operator=(const composite_subscription& other)     = default;
    composite_subscription& operator=(composite_subscription&& other) noexcept = default;

    /**
     * \brief Add any other subscription to this as dependent
     */
    template<constraint::subscription TSub = subscription_base>
    std::weak_ptr<details::subscription_state> add(const TSub &sub = TSub{}) const
    {
        if (static_cast<const subscription_base *>(&sub) == static_cast<const subscription_base *>(this))
            return sub.get_state();

        if (const auto pstate = std::static_pointer_cast<state>(get_state()))
            pstate->add(sub.get_state());
        else
            sub.unsubscribe();
        return sub.get_state();
    }

    /**
     * \brief Add callback/function subscription to this as dependent
     */
    std::weak_ptr<details::subscription_state> add(const callback_subscription &sub) const
    {
        return add<callback_subscription>(sub);
    }

    composite_subscription make_child() const
    {
        composite_subscription ret{};
        ret.add([weak_handle = add(ret), state = std::weak_ptr{std::static_pointer_cast<state>(get_state())}]
        {
            // add cleanup
            if (const auto     locked_state  = state.lock())
                if (const auto locked_handle = weak_handle.lock())
                    locked_state->remove(locked_handle);
        });
        return ret;
    }

    void remove(const subscription_base &sub) const
    {
        if (const auto pstate = std::static_pointer_cast<state>(get_state()))
            pstate->remove(sub.get_state());
    }

    void remove(const std::weak_ptr<details::subscription_state>& sub) const
    {
        if (const auto locked = sub.lock())
            if (const auto pstate = std::static_pointer_cast<state>(get_state()))
                pstate->remove(locked);
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
        state(std::vector<std::shared_ptr<details::subscription_state>>&& deps)
            : m_deps{std::move(deps)} 
            {
                m_deps.erase(std::remove_if(m_deps.begin(), m_deps.end(), [](const auto& ptr){return !ptr;}), m_deps.end());
            }

        void add(std::shared_ptr<details::subscription_state> sub)
        {
            if (!sub || !sub->is_subscribed())
                return;

            while (true)
            {
                DepsState expected{DepsState::None};
                if (m_state.compare_exchange_strong(expected, DepsState::Edit, std::memory_order::acq_rel))
                {
                    m_deps.push_back(std::move(sub));
                    
                    m_state.store(DepsState::None, std::memory_order::release);
                    return;
                }

                if (expected == DepsState::Unsubscribed)
                {
                    sub->unsubscribe();
                    return;
                }
            }
        }

        void remove(const std::shared_ptr<details::subscription_state>& sub)
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
                    std::ranges::for_each(m_deps, &subscription_state::unsubscribe);
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

        std::atomic<DepsState>                                    m_state{DepsState::None};
        std::vector<std::shared_ptr<details::subscription_state>> m_deps{};
    };
};
} // namespace rpp
