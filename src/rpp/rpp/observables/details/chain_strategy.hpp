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
    using base = observable_chain_strategy<TStrategies...>;

public:
    using expected_disposable_strategy = details::observables::deduce_updated_disposable_strategy<TStrategy, typename base::expected_disposable_strategy>;
    using value_type = typename TStrategy::template operator_traits_for_upstream_type<typename base::value_type>::result_type;

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

    template<rpp::constraint::observer_of_type<value_type> Observer>
    void subscribe(Observer&& observer) const
    {
        if constexpr (rpp::constraint::operator_lift_with_disposable_strategy<TStrategy, typename base::value_type, typename base::expected_disposable_strategy>)
            m_strategies.subscribe(m_strategy.template lift_with_disposable_strategy<typename base::value_type, typename base::expected_disposable_strategy>(std::forward<Observer>(observer)));
        else if constexpr (rpp::constraint::operator_lift<TStrategy, typename base::value_type>)
            m_strategies.subscribe(m_strategy.template lift<typename base::value_type>(std::forward<Observer>(observer)));
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
    using expected_disposable_strategy = rpp::details::observables::deduce_disposable_strategy_t<TStrategy>;
    using value_type                   = typename TStrategy::value_type;

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