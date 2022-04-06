// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <rpp/subscriptions/subscription_base.h>
#include <rpp/subscriptions/callback_subscription.h>
#include <rpp/utils/constraints.h>

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
    subscription_base add(const subscription_base& sub) const
    {
        static_cast<state&>(get_state()).add(sub);
        return sub;
    }

    /**
     * \brief Add callback/function subscription to this as dependent
     */
    subscription_base add(const callback_subscription& sub) const
    {
        return add(static_cast<const subscription_base&>(sub));
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
