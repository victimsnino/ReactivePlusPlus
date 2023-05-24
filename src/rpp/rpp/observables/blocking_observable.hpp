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
struct blocking_observer_strategy
{
    mutable std::promise<void> promise;

    constexpr static operators::details::forwarding_on_next_strategy on_next{};

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

    constexpr static operators::details::forwarding_set_upstream_strategy set_upstream{};
    constexpr static operators::details::forwarding_is_disposed_strategy is_disposed{};
    constexpr static operators::details::empty_on_subscribe on_subscribe{};
};

template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class blocking_strategy
{
public:
    blocking_strategy(observable<Type, Strategy>&& observable) : m_original{std::move(observable)}{}
    blocking_strategy(const observable<Type, Strategy>& observable) : m_original{observable}{}

    template<constraint::observer_strategy<Type> ObserverStrategy>
    void subscribe(observer<Type, ObserverStrategy>&& obs) const
    {
        std::promise<void> promise{};
        auto future = promise.get_future();
        m_original.subscribe(observer<Type, rpp::operators::details::operator_strategy_base<Type, observer<Type, ObserverStrategy>, blocking_observer_strategy>>{std::move(obs), std::move(promise)});
        future.wait();
    }

private:
    RPP_NO_UNIQUE_ADDRESS observable<Type, Strategy> m_original;
};
}