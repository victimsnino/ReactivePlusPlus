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
