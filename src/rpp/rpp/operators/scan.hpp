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

#include "rpp/utils/utils.hpp"
#include <rpp/operators/fwd.hpp>
#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <cstddef>

namespace rpp::operators::details
{
template<rpp::constraint::decayed_type InitialValue, rpp::constraint::decayed_type Fn>
struct scan_observer_strategy
{
    RPP_NO_UNIQUE_ADDRESS mutable InitialValue seed;
    RPP_NO_UNIQUE_ADDRESS Fn fn;

    void on_subscribe(const rpp::constraint::observer auto& obs) const
    {
        obs.on_next(seed);
    }

    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        seed = fn(std::move(seed), std::forward<T>(v));
        obs.on_next(utils::as_const(seed));
    }

    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_on_completed_strategy on_completed{};
    constexpr static forwarding_set_upstream_strategy set_upstream{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};

};

template<rpp::constraint::observable TObservable, rpp::constraint::decayed_type InitialValue, std::invocable<InitialValue&&, rpp::utils::extract_observable_type_t<TObservable>> Fn>
using scan_observable = operator_observable<InitialValue, TObservable, scan_observer_strategy<InitialValue, Fn>, InitialValue, Fn>;


template<rpp::constraint::decayed_type InitialValue, rpp::constraint::decayed_type Fn>
struct scan_t
{
    RPP_NO_UNIQUE_ADDRESS InitialValue m_initial_value;
    RPP_NO_UNIQUE_ADDRESS Fn           m_fn;

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, InitialValue&&, rpp::utils::extract_observable_type_t<TObservable>> && std::same_as<InitialValue, std::invoke_result_t<Fn, InitialValue&&, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) const &
    {
        return scan_observable<TObservable, InitialValue, Fn>{std::forward<TObservable>(observable), m_initial_value, m_fn};
    }

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, InitialValue&&, rpp::utils::extract_observable_type_t<TObservable>> && std::same_as<InitialValue, std::invoke_result_t<Fn, InitialValue&&, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) &&
    {
        return scan_observable<TObservable, InitialValue, Fn>{std::forward<TObservable>(observable), std::move(m_initial_value), std::move(m_fn)};
    }
};
}

namespace rpp::operators
{
/**
 * @brief Apply accumulator function for each emission from observable and result of accumulator from previous step and emit (and cache) resulting value
 *
 * @marble scan
 {
     source observable                : +--1-2-3-|
     operator "scan: s=1, (s,x)=>s+x" : +--2-4-7-|
 }
 *
 * @details Acttually this operator applies provided accumulator function to seed and new emission, emits resulting value and updates seed value for next emission
 * @details Initial value would be used as first value from this observable (would be sent during subscription) and initial value for cache
 *
 * @param initial_value initial value for seed which would be sent during subscription and applied for first value from observable. Then it will be replaced with result and etc.
 * @param accumulator function which accepts seed value and new value from observable and return new value of seed. Can accept seed by move-reference.
 *
 * @warning #include <rpp/operators/scan.hpp>
 *
 * @par Example
 * @snippet scan.cpp scan
 * @snippet scan.cpp scan_vector
 *
 * @ingroup transforming_operators
 * @see https://reactivex.io/documentation/operators/scan.html
 */
template<typename InitialValue, typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<std::decay_t<InitialValue>, std::invoke_result_t<Fn, std::decay_t<InitialValue>&&, utils::convertible_to_any>>)
auto scan(InitialValue&& initial_value, Fn&& accumulator)
{
    return details::scan_t<std::decay_t<InitialValue>, std::decay_t<Fn>>{std::forward<InitialValue>(initial_value), std::forward<Fn>(accumulator)};
}
} // namespace rpp::operators