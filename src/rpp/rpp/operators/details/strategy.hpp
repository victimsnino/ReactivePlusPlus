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

#include <rpp/observables/base_observable.hpp>
#include <rpp/sources/fwd.hpp>

#include <functional>
#include <variant>


namespace rpp::operators::details
{
template<constraint::decayed_type T, constraint::observer_strategy<T> ObserverStrategy>
class operator_strategy_base
{
public:
    using observer = base_observer<T, ObserverStrategy>;

    operator_strategy_base(observer&& observer) 
        : m_observer{std::in_place_index_t<0>{}, std::move(observer)} {}
    
    operator_strategy_base(const operator_strategy_base&) = delete;
    operator_strategy_base(operator_strategy_base&& other) noexcept
        : m_observer{std::in_place_index_t<1>{}, std::move(other).get_move_observer()} {}

    operator_strategy_base& operator=(const operator_strategy_base&) = delete;
    operator_strategy_base& operator=(operator_strategy_base&&) = delete;

protected:
    observer& get_observer()
    {
        return std::visit([](auto& v) -> observer& {return v;}, m_observer);
    }

    const observer& get_observer() const
    {
        return std::visit([](auto& v) -> const observer& {return v;}, m_observer);
    }

    void on_next(const T& v) const { get_observer().on_next(v); }
    void on_next(T&& v) const { get_observer().on_next(std::move(v)); }
    void on_error(const std::exception_ptr& err) const { get_observer().on_error(err); }
    void on_completed() const { get_observer().on_completed(); }

private:
    observer&& get_move_observer()
    {
        return std::visit([](auto& v) -> observer&& {return std::move(v);}, m_observer);
    }
private:
    std::variant<std::reference_wrapper<observer>, observer> m_observer{};
};

template<constraint::decayed_type T, 
         constraint::observable_strategy<T> ObservableStrategy, 
         template<typename> typename TargetObserver, 
         typename... Args>
class lift_observable_strategy
{
public:
    template<constraint::decayed_same_as<Args> ...TArgs>
    lift_observable_strategy(const rpp::base_observable<T, ObservableStrategy>& observable, TArgs&&...args)
        : m_observable{observable}
        , m_vals{std::forward<TArgs>(args)...} {}
    
    template<constraint::decayed_same_as<Args> ...TArgs>
    lift_observable_strategy(rpp::base_observable<T, ObservableStrategy>&& observable, TArgs&&...args)
        : m_observable{std::move(observable)}
        , m_vals{std::forward<TArgs>(args)...} {}

    template<constraint::observer_strategy<T> ObserverStrategy>
    void subscribe(base_observer<T, ObserverStrategy>&& observer) const
    {
        m_observable.subscribe(std::apply(
            [&observer](const Args&... vals) {
                return TargetObserver<base_observer<T, ObserverStrategy>>{std::move(observer), vals...};
            },
            m_vals));
    }

private:
    rpp::base_observable<T, ObservableStrategy> m_observable;
    std::tuple<Args...>                         m_vals{};
};
}