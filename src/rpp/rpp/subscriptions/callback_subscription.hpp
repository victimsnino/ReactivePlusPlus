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
#include <rpp/subscriptions/details/subscription_state.hpp>

#include <concepts>
#include <utility>

namespace rpp
{
/**
 * \brief Subscription which invoke callbable during unsubscribe
 */
class callback_subscription final : public subscription_base
{
public:
    template<std::invocable Fn>
    callback_subscription(Fn&& fn)
        : subscription_base{std::make_shared<state<std::decay_t<Fn>>>(std::forward<Fn>(fn))} { }

private:
    template<std::invocable Fn>
    class state final : public details::subscription_state
    {
    public:
        state(const Fn& fn)
            : m_fn{fn} {}

        state(Fn&& fn)
            : m_fn{std::move(fn)} {}

    protected:
        void on_unsubscribe() override
        {
            m_fn();
        }

    private:
        Fn m_fn{};
    };
};
} // namespace rpp
