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
#include <rpp/disposables/base_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/utils/exceptions.hpp>

#include <exception>
#include <stdexcept>
#include <type_traits>

namespace rpp::details
{
class upstream_disposable
{
protected:
    void set_upstream_impl(const disposable_wrapper& d)
    {
        if (!m_upstream.get_original())
            m_upstream = d;
        else
            throw utils::error_set_upstream_calle_twice{};
    }

    void dispose_impl() const
    {
        m_upstream.dispose();
    }

private:
    disposable_wrapper             m_upstream{};
};
class external_disposable_strategy : private upstream_disposable
{
public:
    explicit external_disposable_strategy(disposable_wrapper disposable) : m_external_disposable(std::move(disposable)) {}

    void set_upstream(const disposable_wrapper& d)
    {
        upstream_disposable::set_upstream_impl(d);
        m_external_disposable.add(d.get_original());
    }

    bool is_disposed() const
    {
        return m_external_disposable.is_disposed();
    }

    void dispose() const
    {
        m_external_disposable.dispose();
        upstream_disposable::dispose_impl();
    }

private:
    disposable_wrapper             m_external_disposable{};
};

class local_disposable_strategy : private upstream_disposable
{
public:
    local_disposable_strategy() = default;
    void set_upstream(const disposable_wrapper& d)
    {
        upstream_disposable::set_upstream_impl(d);
    }

    bool is_disposed() const
    {
        return m_is_disposed;
    }

    void dispose() const
    {
        m_is_disposed = true;
        upstream_disposable::dispose_impl();
    }

private:
    mutable bool m_is_disposed{false};
};

struct none_disposable_strategy
{
    static void set_upstream(const disposable_wrapper&) {}
    static bool is_disposed() { return false; }
    static void dispose() {}
};

template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy, typename DisposablesStrategy>
class base_observer_impl
{
protected:
    template<typename... Args>
        requires constraint::is_constructible_from<Strategy, Args&&...>
    explicit base_observer_impl(DisposablesStrategy&& strategy, Args&&... args)
        : m_strategy{std::forward<Args>(args)...}, m_disposable{std::move(strategy)}
    {
    }

    base_observer_impl(const base_observer_impl&)     = default;
    base_observer_impl(base_observer_impl&&) noexcept = default;

public:
    /**
     * @brief Observable calls this method to pass disposable. Observer disposes this disposable WHEN observer wants to unsubscribe.
     */
    void set_upstream(const disposable_wrapper& d)
    {
        m_disposable.set_upstream(d);
        m_strategy.set_upstream(d);
    }

    /**
     * @brief Observable calls this method to check if observer interested or not in emissions
     *
     * @return true if observer disposed and no longer interested in emissions
     * @return false if observer still interested in emissions
     */
    bool is_disposed() const
    {
        return m_disposable.is_disposed() ||  m_strategy.is_disposed();
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
            m_disposable.dispose();
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
            m_disposable.dispose();
        }
    }

private:
    RPP_NO_UNIQUE_ADDRESS Strategy            m_strategy;
    RPP_NO_UNIQUE_ADDRESS DisposablesStrategy m_disposable;
};
}
namespace rpp
{
/**
 * @brief Base class for any observer used in RPP. It handles core callbacks of observers. Objects of this class would
 * be passed to subscribe of observable
 *
 * @warning By default base_observer is not copyable, only movable. If you need to COPY your observer, you need to convert it to rpp::dynamic_observer via rpp::base_observer::as_dynamic
 * @warning Expected that observer would be subscribed only to ONE observable ever. It can keep internal state and track it it was disposed or not. So, subscribing same observer multiple time follows unspecified behavior.
 * @warning If you are passing disposable to ctor, then state of this disposable would be used used (if empty disposable or disposed -> observer is disposed by default)
 *
 * @tparam Type of value this observer can handle
 * @tparam Strategy used to provide logic over observer's callbacks
 */
template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
class base_observer;

template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
class base_observer final : public details::base_observer_impl<Type, Strategy, details::local_disposable_strategy>
{
public:
    template<typename ...Args>
        requires (!constraint::variadic_decayed_same_as<base_observer<Type, Strategy>, Args...> && constraint::is_constructible_from<Strategy, Args&&...>)
    explicit base_observer(Args&& ...args)
        : details::base_observer_impl<Type, Strategy, details::local_disposable_strategy>{details::local_disposable_strategy{}, std::forward<Args>(args)...}
    {}

    base_observer(const base_observer&)     = delete;
    base_observer(base_observer&&) noexcept = default;

    /**
     * @brief Convert current observer to type-erased version. Useful if you need to COPY your observer or to store different observers in same container.
     */
    dynamic_observer<Type> as_dynamic() &&
    {
        return dynamic_observer<Type>{std::move(*this)};
    }
};

template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
class base_observer<Type, details::with_disposable<Strategy>> final
    : public details::base_observer_impl<Type, Strategy, details::external_disposable_strategy>
{
public:
    template<typename ...Args>
        requires (constraint::is_constructible_from<Strategy, Args&&...>)
    explicit base_observer(disposable_wrapper disposable, Args&& ...args)
        : details::base_observer_impl<Type, Strategy, details::external_disposable_strategy>{details::external_disposable_strategy{std::move(disposable)}, std::forward<Args>(args)...}
    {}

    base_observer(const base_observer&)     = delete;
    base_observer(base_observer&&) noexcept = default;

    /**
     * @brief Convert current observer to type-erased version. Useful if you need to COPY your observer or to store different observers in same container.
     */
    dynamic_observer<Type> as_dynamic() &&
    {
        return dynamic_observer<Type>{std::move(*this)};
    }
};

template<constraint::decayed_type Type>
class base_observer<Type, rpp::details::observer::dynamic_strategy<Type>> final
    : public details::base_observer_impl<Type, rpp::details::observer::dynamic_strategy<Type>, details::none_disposable_strategy>
{
public:
    template<constraint::observer_strategy<Type> TStrategy>
        requires (!std::same_as<TStrategy, rpp::details::observer::dynamic_strategy<Type>>)
    explicit base_observer(base_observer<Type, TStrategy>&& other)
        : details::base_observer_impl<Type, rpp::details::observer::dynamic_strategy<Type>, details::none_disposable_strategy>{details::none_disposable_strategy{}, std::move(other)}
    {}

    base_observer(const base_observer&)     = default;
    base_observer(base_observer&&) noexcept = default;

    dynamic_observer<Type> as_dynamic() &&
    {
        return std::move(*this);
    }

    const dynamic_observer<Type>& as_dynamic() &
    {
        return *this;
    }
};


} // namespace rpp
