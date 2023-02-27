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

#include <rpp/observers/interface_observer.hpp>

#include <exception>
#include <utility>

namespace rpp
{
template<constraint::decayed_type Type>
class base_observer : public interface_observer<Type>
{
public:
    base_observer()                         = default;
    base_observer(const base_observer&)     = delete;
    base_observer(base_observer&&) noexcept = default;

    void on_next(const Type& v) const final
    {
        if (m_is_subscribed)
            on_next_impl(v);
    }

    void on_next(Type&& v) const final
    {
        if (m_is_subscribed)
            on_next_impl(std::move(v));
    }

    void on_error(const std::exception_ptr& err) const final
    {
        if (std::exchange(m_is_subscribed, false))
            on_error_impl(err);
    }

    void on_completed() const final
    {
        if (std::exchange(m_is_subscribed, false))
            on_completed_impl();
    }

protected:
    struct internal_copy{};

    base_observer(internal_copy, const base_observer& base_observer)
        : m_is_subscribed{base_observer.m_is_subscribed} {}

    virtual void on_next_impl(const Type& v) const = 0;
    virtual void on_next_impl(Type&& v) const = 0;
    virtual void on_error_impl(const std::exception_ptr& err) const = 0;
    virtual void on_completed_impl() const = 0;

private:
    mutable bool m_is_subscribed{true};
};
} // namespace rpp
