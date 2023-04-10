//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/defs.hpp>
#include <rpp/observers/fwd.hpp>
#include <rpp/observers/dynamic_observer.hpp>

#include <rpp/disposables/composite_disposable.hpp>
#include <exception>
#include <type_traits>

#include <optional>

namespace rpp
{
/**
 * @brief Base class for any observer used in RPP. It handles core callbacks of observers. Objects of this class would
 * be passed to subscribe of observable
 *
 * @warning By default base_observer is not copyable, only movable. If you need to COPY your observer, you need to convert it to rpp::dynamic_observer via rpp::base_observer::as_dynamic
 * @warning Expected that observer would be subscribed only to ONE observable ever. It can keep internal state and track it it was disposed or not. So, subscribing same observer multiple time follows unspecified behavior.
 *
 * @tparam Type of value this observer can handle
 * @tparam Strategy used to provide logic over observer's callbacks
 */
template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
class base_observer final
{
public:
    template<typename ...Args>
        requires constraint::is_constructible_from<Strategy, Args...>
    explicit base_observer(std::optional<composite_disposable> disposable, Args&& ...args)
        : m_upstream{std::move(disposable)}
        , m_strategy{std::forward<Args>(args)...} {}

    template<typename ...Args>
        requires (!constraint::variadic_decayed_same_as<base_observer<Type, Strategy>, Args...> && constraint::is_constructible_from<Strategy, Args&&...>)
    explicit base_observer(Args&& ...args)
        : m_strategy{std::forward<Args>(args)...} {}

    base_observer(base_observer&&) noexcept = default;

    base_observer(const base_observer&) requires (!std::same_as<Strategy, details::observer::dynamic_strategy<Type>>) = delete;
    base_observer(const base_observer&) requires std::same_as<Strategy, details::observer::dynamic_strategy<Type>>  = default;

    /**
     * @brief Observable calls this method to pass disposable to dispose observable IF and WHEN observer want's to unsubscribe.
     */
    void set_upstream(const composite_disposable& d)
    {
        m_strategy.set_upstream(d);

        if (!m_upstream)
            m_upstream = d;
        else
            m_upstream->add(d);
    }

    /**
     * @brief Observable calls this method to check if observer interested or not in emissions
     *
     * @return true if observer disposed and no longer interested in emissions
     * @return false if observer still interested in emissions
     */
    bool is_disposed() const noexcept
    {
        return m_is_disposed || is_upstream_disposed() || m_strategy.is_disposed();
    }
    /**
     * @brief Observable calls this method to notify observer about new value.
     *
     * @note obtains value by const-reference to original object.
     */
    void on_next(const Type& v) const
    {
        try
        {
            if (!is_disposed())
                m_strategy.on_next(v);
        }
        catch(...)
        {
            on_error(std::current_exception());
        }
    }

    /**
     * @brief Observable calls this method to notify observer about new value.
     *
     * @note obtains value by rvalue-reference to original object
     */
    void on_next(Type&& v) const
    {
        try
        {
            if (!is_disposed())
                m_strategy.on_next(std::move(v));
        }
        catch(...)
        {
            on_error(std::current_exception());
        }
    }

    /**
     * @brief Observable calls this method to notify observer about some error during generation next data.
     * @warning Obtaining this of this call means no any further on_next/on_error or on_completed calls from this Observable
     * @param err details of error
     */
    void on_error(const std::exception_ptr& err) const
    {
        if (!is_disposed())
        {
            m_strategy.on_error(err);
            dispose();
        }
    }

    /**
     * @brief Observable calls this method to notify observer about completion of emissions.
     * @warning Obtaining this of this call means no any further on_next/on_error or on_completed calls from this Observable
     */
    void on_completed() const
    {
        if (!is_disposed())
        {
            m_strategy.on_completed();
            dispose();
        }
    }

    /**
     * @brief Convert current observer to type-erased version. Useful if you need to COPY your observer or to store different observers in same container.
     */
    dynamic_observer<Type> as_dynamic() &&
    {
        return dynamic_observer<Type>{std::move(m_upstream), std::move(m_strategy)};
    }

    operator dynamic_observer<Type>() &&
    {
        return std::move(*this).as_dynamic();
    }

private:
    void dispose() const
    {
        m_is_disposed = true;
        if (m_upstream)
            m_upstream->dispose();
    }

    bool is_upstream_disposed() const
    {
        return m_upstream && m_upstream->is_disposed();
    }

private:
    std::optional<composite_disposable> m_upstream{};
    RPP_NO_UNIQUE_ADDRESS Strategy      m_strategy;
    mutable bool                        m_is_disposed = is_upstream_disposed();
};
} // namespace rpp
