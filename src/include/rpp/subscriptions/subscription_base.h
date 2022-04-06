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

#include <rpp/subscriptions/details/subscription_state.h>

#include <atomic>
#include <memory>

namespace rpp
{
/**
 * \brief Base subscription implementation used as base class/interface and core implementation for derrived subscriptions
 */
class subscription_base
{
protected:
    subscription_base(std::shared_ptr<details::subscription_state> state)
        : m_state{std::move(state)} {}

    details::subscription_state& get_state() const { return *m_state; }
public:
    subscription_base()
        : m_state{std::make_shared<details::subscription_state>()} {}

    virtual ~subscription_base() = default;

    /**
     * \brief indicates current status of subscription
     */
    [[nodiscard]] bool is_subscribed() const
    {
        return m_state->is_subscribed();
    }

    /**
     * \brief initiates unsubscription process (if subscribed)
     */
    void unsubscribe() const
    {
        m_state->unsubscribe();
    }

private:
    std::shared_ptr<details::subscription_state> m_state{};
};

} // namespace rpp
