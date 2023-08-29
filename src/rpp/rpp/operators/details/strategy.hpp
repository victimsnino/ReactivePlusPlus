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
#include <rpp/observables/observable.hpp>
#include <rpp/observables/details/chain_strategy.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/tuple.hpp>
#include <rpp/utils/utils.hpp>

#include <exception>
#include <variant>

namespace rpp::operators::details
{
template<typename SubscribeStrategy, rpp::constraint::decayed_type... Args>
class operator_observable_strategy_base
{
public:
    template<rpp::constraint::decayed_same_as<Args> ...TArgs>
    operator_observable_strategy_base(TArgs&&...args)
        : m_vals{std::forward<TArgs>(args)...} {}

    template<rpp::constraint::decayed_type Type, rpp::constraint::observer Observer>
    auto lift(Observer&& observer) const
    {
        return m_vals.apply(&SubscribeStrategy::template apply<Type, Observer, Args...>, std::forward<Observer>(observer));
    }

private:
    RPP_NO_UNIQUE_ADDRESS rpp::utils::tuple<Args...> m_vals{};
};

template<template<typename, typename...> typename Strategy, typename Types>
struct identity_subscribe_strategy;

template<template<typename, typename...> typename Strategy, typename ...Types>
struct identity_subscribe_strategy<Strategy, rpp::utils::types<Types...>>
{
    template<rpp::constraint::decayed_type Type, rpp::constraint::observer Observer, typename ...Args>
    static auto apply(Observer&& observer, const Args&... vals)
    {
        return rpp::observer<Type, Strategy<std::decay_t<Observer>, Types...>>{std::forward<Observer>(observer), vals...};
    }
};

template<template<typename, typename...> typename Strategy, rpp::constraint::decayed_type... Args>
using operator_observable_strategy = operator_observable_strategy_base<identity_subscribe_strategy<Strategy, rpp::utils::types<Args...>>, Args...>;

template<template<typename, typename...> typename Strategy, typename Types = rpp::utils::types<>, rpp::constraint::decayed_type... Args>
using operator_observable_strategy_diffferent_types = operator_observable_strategy_base<identity_subscribe_strategy<Strategy, Types>, Args...>;

template<template<typename, typename, typename...> typename Strategy, typename Types>
struct template_subscribe_strategy;

template<template<typename, typename, typename...> typename Strategy, typename...Types>
struct template_subscribe_strategy<Strategy, rpp::utils::types<Types...>>
{
    template<rpp::constraint::decayed_type Type, rpp::constraint::observer Observer, typename ...Args>
    static auto apply(Observer&& observer, const Args&... vals)
    {
        return rpp::observer<Type, Strategy<Type, std::decay_t<Observer>, Types...>>{std::forward<Observer>(observer), vals...};
    }
};

template<template<typename, typename, typename...> typename Strategy, rpp::constraint::decayed_type... Args>
using template_operator_observable_strategy = operator_observable_strategy_base<template_subscribe_strategy<Strategy, rpp::utils::types<Args...>>, Args...>;

template<template<typename, typename, typename...> typename Strategy, typename Types = rpp::utils::types<>, rpp::constraint::decayed_type... Args>
using template_operator_observable_strategy_different_types = operator_observable_strategy_base<template_subscribe_strategy<Strategy, Types>, Args...>;
}
