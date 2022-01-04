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

#include <atomic>

namespace rpp
{
class subscription
{
public:
    subscription() : m_state{std::make_shared<state>()} {}


    [[nodiscard]] bool is_subscribed() const
    {
        return m_state->is_subscribed.load();
    }

    void unsubscribe() const
    {
        m_state->is_subscribed.store(false);
    }

private:
    struct state
    {
        std::atomic_bool is_subscribed{true};
    };

    std::shared_ptr<state> m_state{};
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
