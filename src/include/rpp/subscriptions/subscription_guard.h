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

namespace rpp
{
/**
 * \brief guard over subscription to auto-unsubscribe during destructor
 */
class subscription_guard
{
public:
    subscription_guard(const subscription_base& sub)
        : m_sub{sub} {}

    subscription_guard(const subscription_guard&) = delete;

    subscription_guard& operator=(const subscription_guard& other)
    {
        m_sub.unsubscribe();
        m_sub = other.m_sub;
        return *this;
    };

    ~subscription_guard()
    {
        m_sub.unsubscribe();
    }

    const subscription_base* operator->() const { return &m_sub; }
private:
    subscription_base m_sub;
};
} // namespace rpp
