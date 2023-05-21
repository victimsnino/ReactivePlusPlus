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

#include <type_traits>

namespace rpp::operators::details
{
template<rpp::constraint::decayed_type Fn>
struct filter_observer_strategy
{
    RPP_NO_UNIQUE_ADDRESS Fn fn;

    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        if (fn(rpp::utils::as_const(v)))
            obs.on_next(std::forward<T>(v));
    }

    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_on_completed_strategy on_completed{};
    constexpr static forwarding_set_upstream_strategy set_upstream{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};
    constexpr static empty_on_subscribe on_subscribe{};
};


template<rpp::constraint::observable TObservable, std::invocable<rpp::utils::extract_observable_type_t<TObservable>> Fn>
using filter_observable = identity_operator_observable<std::decay_t<TObservable>, filter_observer_strategy<Fn>, Fn>;


template<rpp::constraint::decayed_type Fn>
struct filter_t
{
public:
    RPP_NO_UNIQUE_ADDRESS Fn m_fn;

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, rpp::utils::extract_observable_type_t<TObservable>> && std::same_as<bool, std::invoke_result_t<Fn, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) const &
    {
        return filter_observable<TObservable, Fn>{std::forward<TObservable>(observable), m_fn};
    }

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, rpp::utils::extract_observable_type_t<TObservable>> && std::same_as<bool, std::invoke_result_t<Fn, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) &&
    {
        return filter_observable<TObservable, Fn>{std::forward<TObservable>(observable), std::move(m_fn)};
    }
};
}

namespace rpp::operators
{
/**
* @brief Emit only those items from an Observable that satisfies a provided predicate
*
* @marble filter
{
    source observable            : +--1-2-3-4-|
    operator "filter: x=>x%2==0" : +----2---4-|
}
*
* @details Actually this operator just checks if predicate returns true, then forwards emission
*
* @par Performance notes:
* - No any heap allocations at all
* - No any copies/moves of emissions, just passing by const& to predicate and then forwarding
*
* @param predicate is predicate used to check emitted items. true -> items satisfies condition, false -> not
* @warning #include <rpp/operators/filter.hpp>
*
* @par Example:
* @snippet filter.cpp Filter
*
* @ingroup filtering_operators
* @see https://reactivex.io/documentation/operators/filter.html
*/
template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<bool, std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto filter(Fn&& predicate)
{
    return details::filter_t<std::decay_t<Fn>>{std::forward<Fn>(predicate)};
}
} // namespace rpp::operators