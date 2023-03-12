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
#include <memory>
#include <stdexcept>
#
namespace rpp
{
template<constraint::decayed_type Type>
class dynamic_observer final : public interface_observer<Type>
{
public:
    dynamic_observer(std::shared_ptr<interface_observer<Type>> observer)
        : m_observer{std::move(observer)}
    {
        if (!m_observer)
            throw std::invalid_argument{"provided std::shared_ptr<interface_observer<Type>> is nullptr"};
    }

    template<constraint::observer_of_type<Type> TObs>
    dynamic_observer(TObs&& obs)
        : dynamic_observer{std::forward<TObs>(obs).as_dynamic()} {}

    dynamic_observer(const dynamic_observer&)     = default;
    dynamic_observer(dynamic_observer&&) noexcept = default;

    void on_next(const Type& v) const noexcept final
    {
        m_observer->on_next(v);
    }

    void on_next(Type&& v) const noexcept final
    {
        m_observer->on_next(std::move(v));
    }

    void on_error(const std::exception_ptr& err) const noexcept final
    {
        m_observer->on_error(err);
    }

    void on_completed() const noexcept final
    {
        m_observer->on_completed();
    }

    dynamic_observer<Type> as_dynamic() const & noexcept final { return *this; }
    dynamic_observer<Type> as_dynamic() && noexcept final { return std::move(*this); }
private:
    std::shared_ptr<interface_observer<Type>> m_observer{};
};
} // namespace rpp
