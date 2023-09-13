//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/fwd.hpp>
#include <rpp/observers/fwd.hpp>

#include <rpp/defs.hpp>

namespace rpp
{
template<typename TStrategy, typename... TStrategies>
class observable_chain_strategy
{
public:
    using ValueType = typename TStrategy::template ResultValue<typename observable_chain_strategy<TStrategies...>::ValueType>;

    observable_chain_strategy(const TStrategy& strategy, const TStrategies&... strategies)
        : m_strategy(strategy)
        , m_strategies(strategies...)
    {
    }

    observable_chain_strategy(const TStrategy& strategy, const observable_chain_strategy<TStrategies...>& strategies)
        : m_strategy(strategy)
        , m_strategies(strategies)
    {
    }

    template<rpp::constraint::observer Observer>
    void subscribe(Observer&& observer) const
    {
        if constexpr (rpp::constraint::operator_lift<TStrategy, typename observable_chain_strategy<TStrategies...>::ValueType>)
            m_strategies.subscribe(m_strategy.template lift<typename observable_chain_strategy<TStrategies...>::ValueType>(std::forward<Observer>(observer)));
        else
            m_strategy.subscribe(std::forward<Observer>(observer), m_strategies);
    }

private:
    RPP_NO_UNIQUE_ADDRESS TStrategy                                 m_strategy;
    RPP_NO_UNIQUE_ADDRESS observable_chain_strategy<TStrategies...> m_strategies;
};

template<typename TStrategy>
class observable_chain_strategy<TStrategy>
{
public:
    using ValueType = typename TStrategy::ValueType;

    observable_chain_strategy(const TStrategy& strategy)
        : m_strategy(strategy)
    {
    }

    template<rpp::constraint::observer Observer>
    void subscribe(Observer&& observer) const
    {
        m_strategy.subscribe(std::forward<Observer>(observer));
    }

private:
    RPP_NO_UNIQUE_ADDRESS TStrategy m_strategy;
};

template<typename New, typename Old>
struct make_chain_observable
{
    using type = observable_chain_strategy<New, Old>;
};

template<typename New, typename... Args>
struct make_chain_observable<New, observable_chain_strategy<Args...>>
{
    using type = observable_chain_strategy<New, Args...>;
};

template<typename New, typename Old>
using make_chain_observable_t = typename make_chain_observable<New, Old>::type;
} // namespace rpp