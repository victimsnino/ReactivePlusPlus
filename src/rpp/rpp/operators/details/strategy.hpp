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
#include <rpp/sources/fwd.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/observables/base_observable.hpp>
#include <rpp/utils/constraints.hpp>

#include <exception>
#include <functional>
#include <variant>


namespace rpp::operators::details::constraint
{
template<typename S, typename Type>
concept operator_strategy = requires(const S& const_strategy,
                                     S& strategy,
                                     const Type& v,
                                     const disposable_wrapper disposable,
                                     const dynamic_observer<Type>& const_observer,
                                     dynamic_observer<Type>& observer)
{
    const_strategy.on_next(const_observer, v);
    const_strategy.on_next(const_observer, Type{});
    const_strategy.on_error(const_observer, std::exception_ptr{});
    const_strategy.on_completed(const_observer);

    strategy.set_upstream(observer, disposable);
    { strategy.is_disposed() } -> std::same_as<bool>;
};
}
namespace rpp::operators::details
{
template<rpp::constraint::decayed_type T, rpp::constraint::observer TObs, constraint::operator_strategy<rpp::utils::extract_observer_type_t<T>> Strategy>
class operator_strategy_base;

template<rpp::constraint::decayed_type T, rpp::constraint::decayed_type TT, rpp::constraint::observer_strategy<TT> ObserverStrategy, constraint::operator_strategy<TT> Strategy>
class operator_strategy_base<T, rpp::base_observer<TT, ObserverStrategy>, Strategy>
{
public:
    using observer = base_observer<TT, ObserverStrategy>;

    /**
     * @brief Construct a new operator strategy for passed observer
     * @warning Passed observer would not be moved inside, only rvalue reference saved inside. Actual move happens ONLY in case of move of this strategy
     */
    template<typename...Args>
    operator_strategy_base(observer&& observer, Args&&...args)
        : m_observer{std::move(observer)}
        , m_strategy{std::forward<Args>(args)...} {}

    operator_strategy_base(const operator_strategy_base&) = delete;
    operator_strategy_base(operator_strategy_base&& other) noexcept = default;

    operator_strategy_base& operator=(const operator_strategy_base&) = delete;
    operator_strategy_base& operator=(operator_strategy_base&&) = delete;

    ~operator_strategy_base() noexcept = default;

    void set_upstream(const disposable_wrapper& d)     { m_strategy.set_upstream(m_observer, d); }
    bool is_disposed() const                           { return m_strategy.is_disposed() || m_observer.is_disposed(); }

    void on_next(const T& v) const                     { m_strategy.on_next(m_observer, v); }
    void on_next(T&& v) const                          { m_strategy.on_next(m_observer, std::move(v)); }
    void on_error(const std::exception_ptr& err) const { m_strategy.on_error(m_observer, err); }
    void on_completed() const                          { m_strategy.on_completed(m_observer); }

private:
    observer                       m_observer;
    RPP_NO_UNIQUE_ADDRESS Strategy m_strategy;
};

struct forwarding_on_next_strategy
{
    template<typename T>
    void operator()(const rpp::constraint::observer auto& obs, T&& v) const
    {
        obs.on_next(std::forward<T>(v));
    }
};

struct forwarding_on_error_strategy
{
    void operator()(const rpp::constraint::observer auto & obs, const std::exception_ptr& err) const
    {
        obs.on_error(err);
    }
};

struct forwarding_on_completed_strategy
{
    void operator()(const rpp::constraint::observer auto& obs) const
    {
        obs.on_completed();
    }
};

struct forwarding_set_upstream_strategy
{
    template<rpp::constraint::decayed_type T, rpp::constraint::observer_strategy<T> ObserverStrategy>
    void operator()(rpp::base_observer<T, ObserverStrategy>& observer, const rpp::disposable_wrapper& d) const {observer.set_upstream(d); }
};

struct forwarding_is_disposed_strategy
{
    bool operator()() const {return false; }
};

template<rpp::constraint::observable Observable,
         rpp::constraint::decayed_type T,
         constraint::operator_strategy<rpp::utils::extract_observable_type_t<Observable>> Strategy,
         typename... Args>
    requires (rpp::constraint::is_constructible_from<Strategy, Args...> && rpp::constraint::decayed_type<Observable> && (rpp::constraint::decayed_type<Args> && ...))
class operator_observable_strategy
{
public:
    template<rpp::constraint::decayed_same_as<Args> ...TArgs>
    operator_observable_strategy(const Observable& observable, TArgs&&...args)
        : m_observable{observable}
        , m_vals{std::forward<TArgs>(args)...} {}

    template<rpp::constraint::decayed_same_as<Args> ...TArgs>
    operator_observable_strategy(Observable&& observable, TArgs&&...args)
        : m_observable{std::move(observable)}
        , m_vals{std::forward<TArgs>(args)...} {}

    template<rpp::constraint::observer_strategy<T> ObserverStrategy>
    void subscribe(base_observer<T, ObserverStrategy>&& observer) const
    {
       std::apply([&observer, this](const Args&... vals)
                  {
                       m_observable.subscribe(base_observer<rpp::utils::extract_observable_type_t<Observable>,
                                                            operator_strategy_base<rpp::utils::extract_observable_type_t<Observable>, base_observer<T, ObserverStrategy>, Strategy>>{std::move(observer), vals...});
                  },
                  m_vals);
    }

private:
    RPP_NO_UNIQUE_ADDRESS Observable          m_observable;
    RPP_NO_UNIQUE_ADDRESS std::tuple<Args...> m_vals{};
};

template<rpp::constraint::observable Observable,
         constraint::operator_strategy<rpp::utils::extract_observable_type_t<Observable>> Strategy,
         typename... Args>
using identity_operator_observable = rpp::base_observable<rpp::utils::extract_observable_type_t<Observable>,
                                                 operator_observable_strategy<std::decay_t<Observable>, rpp::utils::extract_observable_type_t<Observable>, Strategy, Args...>>;

template<rpp::constraint::decayed_type T,
         rpp::constraint::observable Observable,
         constraint::operator_strategy<rpp::utils::extract_observable_type_t<Observable>> Strategy,
         typename... Args>
using operator_observable = rpp::base_observable<T,
                                                 operator_observable_strategy<std::decay_t<Observable>, T, Strategy, Args...>>;
}
