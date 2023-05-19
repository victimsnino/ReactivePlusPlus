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
#include <cstddef>

namespace rpp::operators::details
{
template<rpp::constraint::decayed_type Fn>
struct map_observer_strategy
{
    RPP_NO_UNIQUE_ADDRESS Fn fn;

    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        obs.on_next(fn(std::forward<T>(v)));
    }

    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_on_completed_strategy on_completed{};
    constexpr static forwarding_set_upstream_strategy set_upstream{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};
    constexpr static empty_on_subscribe on_subscribe{};

};

template<rpp::constraint::observable TObservable, std::invocable<rpp::utils::extract_observable_type_t<TObservable>> Fn>
using map_observable = operator_observable<std::invoke_result_t<Fn, rpp::utils::extract_observable_type_t<TObservable>>,
                                           TObservable,
                                           map_observer_strategy<Fn>,
                                           Fn>;


template<rpp::constraint::decayed_type Fn>
struct map_t
{
    RPP_NO_UNIQUE_ADDRESS Fn m_fn;

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, rpp::utils::extract_observable_type_t<TObservable>> && !std::same_as<void, std::invoke_result_t<Fn, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) const &
    {
        return map_observable<TObservable, Fn>{std::forward<TObservable>(observable), m_fn};
    }

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, rpp::utils::extract_observable_type_t<TObservable>> && !std::same_as<void, std::invoke_result_t<Fn, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) &&
    {
        return map_observable<TObservable, Fn>{std::forward<TObservable>(observable), std::move(m_fn)};
    }
};
}

namespace rpp::operators
{
/**
 * @brief Transforms the items emitted by an Observable via applying a function to each item and emitting result
 * @note The Map operator can keep same type of value or change it to some another type.
 *
 * @marble map
 {
     source observable       : +--1   -2   --3   -|
     operator "map: x=>x+10" : +--(11)-(12)--(13)-|
 }
 *
 * @details Actually this operator just applies callable to each obtained emission and emit resulting value
 *
 * @par Performance notes:
 * - No any heap allocations at all
 * - No any copies/moves of emissions, just forwarding to callable
 *
 * @param callable is callable used to provide this transformation. Should accept `Type` of original observable and return type for new observable
 * @warning #include <rpp/operators/map.hpp>
 *
 * @par Example with same type:
 * @snippet map.cpp Same type
 *
 * @par Example with changed type:
 * @snippet map.cpp Changed type
 *
 * @ingroup transforming_operators
 * @see https://reactivex.io/documentation/operators/map.html
 */
template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || !std::same_as<void, std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto map(Fn&& callable)
{
    return details::map_t<std::decay_t<Fn>>{std::forward<Fn>(callable)};
}
} // namespace rpp::operators