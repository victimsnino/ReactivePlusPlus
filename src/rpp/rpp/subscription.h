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

#include <rpp/utils/overloaded.h>

#include <atomic>
#include <memory>
#include <variant>

namespace rpp
{
namespace details
{
    struct subscription_state
    {
        subscription_state() = default;

        subscription_state(const subscription_state& other)
            : is_subscribed{other.is_subscribed.load()} {}

        subscription_state(subscription_state&& other) noexcept
            : is_subscribed{other.is_subscribed.load()} {}

        std::atomic_bool is_subscribed{true};
    };
} // namespace details
class subscription
{
public:
    subscription(const details::subscription_state& state) : m_state{state} {}
    subscription(const std::shared_ptr<details::subscription_state>& state) : m_state{state} {}

    [[nodiscard]] bool is_subscribed() const
    {
        return get_state().is_subscribed.load();
    }

    void unsubscribe() const
    {
        get_state().is_subscribed.store(false);
    }

    subscription as_shared() const
    {
        if (std::holds_alternative<std::shared_ptr<details::subscription_state>>(m_state))
            return *this;
        return std::make_shared<details::subscription_state>(std::get<details::subscription_state>(m_state));
    }

private:
    details::subscription_state& get_state() const
    {
        return std::visit(utils::overloaded
                          {
                              [](details::subscription_state& state) -> details::subscription_state&
                              {
                                  return state;
                              },
                              [](const std::shared_ptr<details::subscription_state>& state_ptr) ->details::subscription_state&
                              {
                                  return *state_ptr;
                              }
                          },
                          m_state);
    }

private:
    mutable std::variant<details::subscription_state, std::shared_ptr<details::subscription_state>> m_state{};
};

template<typename Subscription>
class subscription_guard
{
public:
    subscription_guard(const Subscription& sub)
        : m_sub{sub} {}

    ~subscription_guard() { m_sub.unsubscribe(); }
private:
    const Subscription& m_sub;
};
} // namespace rpp
