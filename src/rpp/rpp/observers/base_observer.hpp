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

#include <rpp/disposables/composite_disposable.hpp>
#include <type_traits>

namespace rpp
{
template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
class base_observer final
{
public:
    template<typename ...Args>
        requires (constraint::is_constructible_from<Strategy, Args&&...> || std::is_trivially_constructible_v<Strategy, Args&&...>)
    base_observer(composite_disposable disposable, Args&& ...args)
        : m_strategy{std::forward<Args>(args)...}
        , m_upstream{std::move(disposable)} {}

    template<typename ...Args>
        requires (constraint::is_constructible_from<Strategy, Args&&...> || std::is_trivially_constructible_v<Strategy, Args&&...>)
    base_observer(Args&& ...args)
        : m_strategy{std::forward<Args>(args)...}
        , m_upstream{composite_disposable::empty()} {}

    base_observer(base_observer&&) noexcept = default;

    base_observer(const base_observer&) requires (!std::same_as<Strategy, details::dynamic_strategy<Type>>) = delete;
    base_observer(const base_observer&) requires std::same_as<Strategy, details::dynamic_strategy<Type>>  = default;

    void set_upstream(const composite_disposable& d)
    {
        m_strategy.set_upstream(d);

        if (m_upstream.is_empty())
            m_upstream = d;
        else
            m_upstream.add(d);
    }

    [[nodiscard]] bool is_disposed() const noexcept
    {
        return (!m_upstream.is_empty() && m_upstream.is_disposed()) || m_strategy->is_disposed();
    }
    /**
     * @brief Observable calls this methods to notify observer about new value.
     *
     * @note obtains value by const-reference to original object.
     */
    void on_next(const Type& v) const
    {
        m_strategy.on_next(v);
    }

    /**
     * @brief Observable calls this methods to notify observer about new value.
     *
     * @note obtains value by rvalue-reference to original object
     */
    void on_next(Type&& v) const
    {
        m_strategy.on_next(std::move(v));
    }

    /**
     * @brief Observable calls this method to notify observer about some error during generation next data.
     * @warning Obtaining this of this call means no any further on_next/on_error or on_completed calls from this Observable
     * @param err details of error
     */
    void on_error(const std::exception_ptr& err) const
    {
        m_strategy.on_error(err);
        m_upstream.dispose();
    }

    /**
     * @brief Observable calls this method to notify observer about completion of emissions.
     * @warning Obtaining this of this call means no any further on_next/on_error or on_completed calls from this Observable
     */
    void on_completed() const
    {
        m_strategy.on_completed();
        m_upstream.dispose();
    }

    /**
     * \brief Convert current observer to type-erased version. Useful if you need to COPY your observer or to store different observers in same container.
     */
    dynamic_observer<Type> as_dynamic() &&
    {
        return {std::move(m_strategy)};
    }

private:
    RPP_NO_UNIQUE_ADDRESS Strategy m_strategy;
    composite_disposable           m_upstream{};
};
} // namespace rpp
