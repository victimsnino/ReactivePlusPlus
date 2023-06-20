//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observers/fwd.hpp>

namespace rpp
{
template<typename TStrategy, typename... TStrategies>
class observable_chain_strategy
{
public:
    observable_chain_strategy(const TStrategy& strategy, const TStrategies& ...strategies)
        : m_strategy(strategy)
        , m_strategies(strategies...)
    {}

    observable_chain_strategy(const TStrategy& strategy, const observable_chain_strategy<TStrategies...>& strategies)
        : m_strategy(strategy)
        , m_strategies(strategies)
    {}

    using Type = typename TStrategy::template Result<typename observable_chain_strategy<TStrategies...>::Type>;

    template<rpp::constraint::observer Observer>
    void subscribe(Observer&& observer) const
    {
        m_strategy.subscribe(std::forward<Observer>(observer), m_strategies);
    }

private:
    TStrategy                                 m_strategy;
    observable_chain_strategy<TStrategies...> m_strategies;
};

template<typename TStrategy>
class observable_chain_strategy<TStrategy>
{
public:
    observable_chain_strategy(const TStrategy& strategy)
        : m_strategy(strategy)
    {}

    using Type = typename TStrategy::Type;

    template<rpp::constraint::observer Observer>
    void subscribe(Observer&& observer) const
    {
        m_strategy.subscribe(std::forward<Observer>(observer));
    }

private:
    TStrategy m_strategy;
};

template<typename New, typename Old>
struct make_chain_observable
{
    using type = observable_chain_strategy<New, Old>;
};

template<typename New, typename ...Args>
struct make_chain_observable<New, observable_chain_strategy<Args...>>
{
    using type = observable_chain_strategy<New, Args...>;
};

template<typename New, typename Old>
using make_chain_observable_t = typename make_chain_observable<New, Old>::type;
} // namespace rpp