//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/defs.hpp>
#include <rpp/observables/fwd.hpp>
#include <rpp/operators/details/strategy.hpp>

#include <future>

namespace rpp::details::observables
{
template<rpp::constraint::observer TObserver>
struct blocking_observer_strategy
{
    RPP_NO_UNIQUE_ADDRESS TObserver observer;
    mutable std::promise<void>      promise;

    void on_error(const rpp::constraint::observer auto & obs, const std::exception_ptr& err) const
    {
        obs.on_error(err);
        promise.set_value();
    }

    void on_completed(const rpp::constraint::observer auto& obs) const
    {
        obs.on_completed();
        promise.set_value();
    }

    template<typename T>
    void on_next(T&& v) const                          { observer.on_next(std::forward<T>(v)); }

    bool is_disposed() const                           { return observer.is_disposed(); }
};

template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class blocking_strategy
{
public:
    using ValueType = Type;
    
    blocking_strategy(observable<Type, Strategy>&& observable) : m_original{std::move(observable)}{}
    blocking_strategy(const observable<Type, Strategy>& observable) : m_original{observable}{}

    template<constraint::observer_strategy<Type> ObserverStrategy>
    void subscribe(observer<Type, ObserverStrategy>&& obs) const
    {
        std::promise<void> promise{};
        auto future = promise.get_future();
        m_original.subscribe(observer<Type, blocking_observer_strategy<observer<Type, ObserverStrategy>>>{std::move(obs), std::move(promise)});
        future.wait();
    }

private:
    RPP_NO_UNIQUE_ADDRESS observable<Type, Strategy> m_original;
};
}