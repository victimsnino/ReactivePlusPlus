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

#include "rpp/disposables/composite_disposable.hpp"
#include "rpp/observables/fwd.hpp"
#include "rpp/utils/constraints.hpp"
#include <rpp/observables/base_observable.hpp>
#include <rpp/observers/fwd.hpp>
#include <rpp/sources/fwd.hpp>

#include <exception>
#include <functional>
#include <variant>


namespace rpp::operators::details::constraint
{
template<typename S, typename Type>
concept operator_strategy = requires(const S& const_strategy,
                                     S& strategy,
                                     const Type& v,
                                     const composite_disposable disposable,
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
template<rpp::constraint::observer T, constraint::operator_strategy<T> Strategy>
class operator_strategy_base;

template<rpp::constraint::decayed_type T, rpp::constraint::observer_strategy<T> ObserverStrategy, constraint::operator_strategy<T> Strategy>
class operator_strategy_base<rpp::base_observer<T, ObserverStrategy>, Strategy>
{
public:
    using observer = base_observer<T, ObserverStrategy>;

    template<typename...Args>
    operator_strategy_base(observer&& observer, Args&&...args)
        : m_observer{std::in_place_index<0>, observer}
        , m_strategy{std::forward<Args>(args)...} {}

    operator_strategy_base(const operator_strategy_base&) = delete;
    operator_strategy_base(operator_strategy_base&& other) noexcept
        : m_observer{std::in_place_index<1>, std::move(other).get_move_observer()} {}

    operator_strategy_base& operator=(const operator_strategy_base&) = delete;
    operator_strategy_base& operator=(operator_strategy_base&&) = delete;

    void set_upstream(const composite_disposable& d)   { m_strategy.set_upstream(get_observer(), d); }
    bool is_disposed() const noexcept                  { return m_strategy.is_disposed() || get_observer().is_disposed(); }

    void on_next(const T& v) const                     { m_strategy.on_next(get_observer(), v); }
    void on_next(T&& v) const                          { m_strategy.on_next(get_observer(), std::move(v)); }
    void on_error(const std::exception_ptr& err) const { m_strategy.on_error(get_observer(), err); }
    void on_completed() const                          { m_strategy.on_completed(get_observer()); }

private:
    observer&& get_move_observer()
    {
        return std::visit([](auto& v) -> observer&& {return std::move(v);}, m_observer);
    }

    observer& get_observer()
    {
        return std::visit([](auto& v) -> observer& {return v;}, m_observer);
    }

    const observer& get_observer() const
    {
        return std::visit([](auto& v) -> const observer& {return v;}, m_observer);
    }
private:
    std::variant<std::reference_wrapper<observer>, observer> m_observer;
    Strategy                                                 m_strategy;
};

struct forwarding_on_next_strategy
{
    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        obs.on_next(std::forward<T>(v));
    }
};

struct forwarding_on_error_strategy
{
    void on_error(const rpp::constraint::observer auto & obs, const std::exception_ptr& err) const
    {
        obs.on_error(err);
    }
};

struct forwarding_on_completed_strategy
{
    void on_completed(const rpp::constraint::observer auto& obs) const
    {
        obs.on_completed();
    }
};

struct forwarding_disposable_strategy
{
    template<rpp::constraint::decayed_type T, rpp::constraint::observer_strategy<T> ObserverStrategy>
    static void set_upstream(rpp::base_observer<T, ObserverStrategy>& observer, const rpp::composite_disposable& d) {observer.set_upstream(d); }
    static bool is_disposed() {return false; }
};

template<rpp::constraint::observable Observable,
         constraint::operator_strategy<rpp::utils::extract_observable_type_t<Observable>> Strategy,
         typename... Args>
    requires (rpp::constraint::is_constructible_from<Strategy, Args...> && rpp::constraint::decayed_type<Observable> && (rpp::constraint::decayed_type<Args> && ...))
class operator_observable_strategy
{
    using T = rpp::utils::extract_observable_type_t<Observable>;
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
        m_observable.subscribe(std::apply([&observer](const Args&... vals)
                                          {
                                            return base_observer<T, operator_strategy_base<base_observer<T, ObserverStrategy>, Strategy>>{std::move(observer), vals...};
                                          },
                                          m_vals));
    }

private:
    Observable          m_observable;
    std::tuple<Args...> m_vals{};
};

template<rpp::constraint::observable Observable,
         constraint::operator_strategy<rpp::utils::extract_observable_type_t<Observable>> Strategy,
         typename... Args>
using operator_observable = rpp::base_observable<rpp::utils::extract_observable_type_t<Observable>,
                                                 operator_observable_strategy<std::decay_t<Observable>, Strategy, Args...>>;
}