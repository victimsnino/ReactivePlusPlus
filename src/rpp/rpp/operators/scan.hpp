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

#include "rpp/utils/constraints.hpp"
#include "rpp/utils/utils.hpp"

#include <cstddef>
#include <optional>

namespace rpp::operators::details
{
template<rpp::constraint::observer TObserver, rpp::constraint::decayed_type Seed, rpp::constraint::decayed_type Fn>
struct scan_observer_strategy
{
    using PreferredDisposableStrategy = rpp::details::observers::none_disposable_strategy;

    RPP_NO_UNIQUE_ADDRESS TObserver    observer;
    RPP_NO_UNIQUE_ADDRESS mutable Seed seed;
    RPP_NO_UNIQUE_ADDRESS Fn           fn;

    RPP_CALL_DURING_CONSTRUCTION(
    {
        observer.on_next(utils::as_const(seed));
    });

    template<typename T>
    void on_next(T&& v) const
    {
        seed = fn(std::move(seed), std::forward<T>(v));
        observer.on_next(utils::as_const(seed));
    }

    void on_error(const std::exception_ptr& err) const { observer.on_error(err); }

    void on_completed() const { observer.on_completed(); }

    void set_upstream(const disposable_wrapper& d) { observer.set_upstream(d); }

    bool is_disposed() const { return observer.is_disposed(); }
};

template<rpp::constraint::decayed_type InitialValue, rpp::constraint::decayed_type Fn>
struct scan_t : public operators::details::operator_observable_strategy<scan_observer_strategy, InitialValue, Fn>
{
    using operators::details::operator_observable_strategy<scan_observer_strategy, InitialValue, Fn>::operator_observable_strategy;

    template<rpp::constraint::decayed_type T>
        requires std::is_invocable_r_v<InitialValue, Fn, InitialValue&&, T>
    using ResultValue = InitialValue;
};

template<rpp::constraint::decayed_type Seed, rpp::constraint::observer TObserver, rpp::constraint::decayed_type Fn>
struct scan_no_seed_observer_strategy
{
    RPP_NO_UNIQUE_ADDRESS TObserver observer;
    RPP_NO_UNIQUE_ADDRESS Fn        fn;
    mutable std::optional<Seed>     seed{};

    template<rpp::constraint::decayed_same_as<Seed> T>
    void on_next(T&& v) const
    {
        if (seed)
            seed = fn(std::move(seed).value(), std::forward<T>(v));
        else
            seed = std::forward<T>(v);

        observer.on_next(utils::as_const(seed.value()));
    }

    void on_error(const std::exception_ptr& err) const { observer.on_error(err); }

    void on_completed() const { observer.on_completed(); }

    void set_upstream(const disposable_wrapper& d) { observer.set_upstream(d); }

    bool is_disposed() const { return observer.is_disposed(); }
};

template<rpp::constraint::decayed_type Fn>
struct scan_no_seed_t : public operators::details::template_operator_observable_strategy<scan_no_seed_observer_strategy, Fn>
{
    template<rpp::constraint::decayed_type T>
        requires std::is_invocable_r_v<T, Fn, T&&, T>
    using ResultValue = T;
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
     operator "scan: s=1, (s,x)=>s+x" : +1-2-4-7-|
 }
 *
 * @details Actually this operator applies provided accumulator function to seed and new emission, emits resulting value and updates seed value for next emission
 * @warning Initial value would be used as first value from this observable (would be sent during subscription) and initial value for cache
 *
 * @par Performance notes:
 * - No any heap allocations at all
 * - Keep actual seed/cache inside observable and updating it every emission
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
    requires (!utils::is_not_template_callable<Fn> || std::same_as<std::decay_t<InitialValue>, std::invoke_result_t<Fn, std::decay_t<InitialValue> &&, rpp::utils::convertible_to_any>>)
auto scan(InitialValue&& initial_value, Fn&& accumulator)
{
    return details::scan_t<std::decay_t<InitialValue>, std::decay_t<Fn>>{std::forward<InitialValue>(initial_value), std::forward<Fn>(accumulator)};
}

/**
 * @brief Apply accumulator function for each emission from observable and result of accumulator from previous step and emit (and cache) resulting value
 *
 * @marble scan_no_seed
 {
     source observable           : +--1-2-3-|
     operator "scan: (s,x)=>s+x" : +--1-3-6-|
 }
 *
 * @details Actually this operator applies provided accumulator function to seed and new emission, emits resulting value and updates seed value for next emission
 * @warning There is no initial value for seed, so, first value would be used as seed value and forwarded as is.
 *
 * @par Performance notes:
 * - No any heap allocations at all
 * - Keep actual seed/cache inside observable and updating it every emission
 *
 * @param accumulator function which accepts seed value and new value from observable and return new value of seed. Can accept seed by move-reference.
 *
 * @warning #include <rpp/operators/scan.hpp>
 *
 * @par Example
 * @snippet scan.cpp scan_no_seed
 *
 * @ingroup transforming_operators
 * @see https://reactivex.io/documentation/operators/scan.html
 */
template<typename Fn>
auto scan(Fn&& accumulator)
{
    return details::scan_no_seed_t<std::decay_t<Fn>>{std::forward<Fn>(accumulator)};
}
} // namespace rpp::operators