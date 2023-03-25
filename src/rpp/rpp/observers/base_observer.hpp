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

#include <rpp/defs.hpp>
#include <rpp/observers/fwd.hpp>

namespace rpp
{
template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
class base_observer final
{
public:
    base_observer(const Strategy& strategy)
        : m_strategy{strategy} {}

    base_observer(Strategy&& strategy)
        : m_strategy{std::move(strategy)} {}

    template<typename ...Args>
        requires std::constructible_from<Strategy, Args...>
    base_observer(Args&& ...args) 
        : m_strategy{std::forward<Args>(args)...} {}

    base_observer(const base_observer&)     = delete;
    base_observer(base_observer&&) noexcept = default;

    void on_next(const Type& v) const noexcept
    {
        m_strategy.on_next(v);
    }

    void on_next(Type&& v) const noexcept
    {
        m_strategy.on_next(std::move(v));
    }

    void on_error(const std::exception_ptr& err) const noexcept
    {
        m_strategy.on_error(err);
    }

    void on_completed() const noexcept
    {
        m_strategy.on_completed();
    }

    dynamic_observer<Type> as_dynamic() && noexcept
    {
        return {std::move(m_strategy)};
    }

private:
    RPP_NO_UNIQUE_ADDRESS Strategy m_strategy;
};
} // namespace rpp
