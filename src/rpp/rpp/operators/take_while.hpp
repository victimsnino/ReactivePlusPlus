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

#include <rpp/operators/fwd.hpp>
#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>

namespace rpp::operators::details
{
template<rpp::constraint::decayed_type Fn>
struct RPP_EMPTY_BASES take_while_observer_strategy
{
    RPP_NO_UNIQUE_ADDRESS Fn fn;

    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        if (fn(rpp::utils::as_const(v)))
            obs.on_next(std::forward<T>(v));
        else
            obs.on_completed();
    }

    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_on_completed_strategy on_completed{};
    constexpr static forwarding_set_upstream_strategy set_upstream{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};
};


template<rpp::constraint::observable TObservable, std::invocable<rpp::utils::extract_observable_type_t<TObservable>> Fn>
using take_while_observable = identity_operator_observable<std::decay_t<TObservable>, take_while_observer_strategy<Fn>, Fn>;


template<rpp::constraint::decayed_type Fn>
struct take_while_t
{
public:
    RPP_NO_UNIQUE_ADDRESS Fn m_fn;

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, rpp::utils::extract_observable_type_t<TObservable>> && std::same_as<bool, std::invoke_result_t<Fn, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) const &
    {
        return take_while_observable<TObservable, Fn>{std::forward<TObservable>(observable), m_fn};
    }

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, rpp::utils::extract_observable_type_t<TObservable>> && std::same_as<bool, std::invoke_result_t<Fn, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) &&
    {
        return take_while_observable<TObservable, Fn>{std::forward<TObservable>(observable), std::move(m_fn)};
    }
};
}
namespace rpp::operators
{
/**
 * @brief Sends items from observable while items are satisfy predicate. When condition becomes false -> sends `on_completed`
 *
 * @marble take_while
 {
     source observable                : +--1-2-3-4-5-6-|
     operator "take_while: x => x!=3" : +--1-2-|
 }
 *
 * @details Actually this operator just emits values while predicate returns true
 *
 * @param predicate is predicate used to check items
 * @return new specific_observable with the take_while operator as most recent operator.
 * @warning #include <rpp/operators/take_while.hpp>
 * 
 * @par Example:
 * @snippet take_while.cpp take_while
 *
 * @ingroup conditional_operators
 * @see https://reactivex.io/documentation/operators/takewhile.html
 */
template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<bool, std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto take_while(Fn&& callable)
{
    return details::take_while_t<std::decay_t<Fn>>{std::forward<Fn>(callable)};
}
} // namespace rpp::operators