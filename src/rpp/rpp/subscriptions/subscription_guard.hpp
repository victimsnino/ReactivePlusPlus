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
    }

    void reset(const subscription_base& other)
    {
        m_sub.unsubscribe();
        m_sub = other;
    }

    ~subscription_guard()
    {
        m_sub.unsubscribe();
    }

    const subscription_base* operator->() const { return &m_sub; }
    const subscription_base& operator*() const { return m_sub; }
private:
    subscription_base m_sub;
};
} // namespace rpp
