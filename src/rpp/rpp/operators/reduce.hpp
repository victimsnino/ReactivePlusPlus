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
template<rpp::constraint::observer TObserver, rpp::constraint::decayed_type Accumulator>
struct reduce_observer_strategy
{
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;
    using Seed = rpp::utils::extract_observer_type_t<TObserver>;

    RPP_NO_UNIQUE_ADDRESS TObserver    observer;
    RPP_NO_UNIQUE_ADDRESS mutable Seed seed;
    RPP_NO_UNIQUE_ADDRESS Accumulator  accumulator;

    template<typename T>
    void on_next(T&& v) const
    {
        seed = accumulator(std::move(seed), std::forward<T>(v));
    }

    void on_error(const std::exception_ptr& err) const { observer.on_error(err); }

    void on_completed() const
    {
        observer.on_next(std::move(seed));
        observer.on_completed();
    }

    void set_upstream(const disposable_wrapper& d) { observer.set_upstream(d); }

    bool is_disposed() const { return observer.is_disposed(); }
};

template<rpp::constraint::decayed_type Seed, rpp::constraint::decayed_type Accumulator>
struct reduce_t : public operators::details::operator_observable_strategy_diffferent_types<reduce_observer_strategy, rpp::utils::types<Accumulator>, Seed, Accumulator>
{
    using operators::details::operator_observable_strategy_diffferent_types<reduce_observer_strategy, rpp::utils::types<Accumulator>, Seed, Accumulator>::operator_observable_strategy_diffferent_types;

    template<rpp::constraint::decayed_type T>
        requires std::is_invocable_r_v<Seed, Accumulator, Seed&&, T>
    using result_value = Seed;
};

template<rpp::constraint::observer TObserver, rpp::constraint::decayed_type Accumulator>
struct reduce_no_seed_observer_strategy
{
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;
    using Seed = rpp::utils::extract_observer_type_t<TObserver>;

    RPP_NO_UNIQUE_ADDRESS TObserver   observer;
    RPP_NO_UNIQUE_ADDRESS Accumulator accumulator;
    mutable std::optional<Seed>       seed{};

    template<typename T>
    void on_next(T&& v) const
    {
        if (seed.has_value())
            seed = accumulator(std::move(seed).value(), std::forward<T>(v));
        else
            seed = std::forward<T>(v);
    }

    void on_error(const std::exception_ptr& err) const { observer.on_error(err); }

    void on_completed() const
    {
        observer.on_next(std::move(seed).value());
        observer.on_completed();
    }

    void set_upstream(const disposable_wrapper& d) { observer.set_upstream(d); }

    bool is_disposed() const { return observer.is_disposed(); }
};

template<rpp::constraint::decayed_type Accumulator>
struct reduce_no_seed_t : public operators::details::operator_observable_strategy<reduce_no_seed_observer_strategy, Accumulator>
{
    template<rpp::constraint::decayed_type T>
        requires std::is_invocable_r_v<T, Accumulator, T&&, T>
    using result_value = T;
};
}

namespace rpp::operators
{
/**
 * @brief Apply a function to each item emitted by an Observable, sequentially, and emit the final value
 *
 * @marble reduce
 {
     source observable                : +--1-2-3--|
     operator "reduce: s=10, (s,x)=>s+x" : +------16|
 }
 *
 * @param initial_value initial value for seed
 * @param accumulator function which accepts seed value and new value from observable and return new value of seed. Can accept seed by move-reference.
 *
 * @warning #include <rpp/operators/reduce.hpp>
 *
 * @par Example
 * @snippet reduce.cpp reduce
 *
 * @ingroup aggregate_operators
 * @see https://reactivex.io/documentation/operators/reduce.html
 */
template<typename Seed, typename Accumulator>
    requires (!utils::is_not_template_callable<Accumulator> || std::same_as<std::decay_t<Seed>, std::invoke_result_t<Accumulator, std::decay_t<Seed>&&, rpp::utils::convertible_to_any>>)
auto reduce(Seed&& seed, Accumulator&& accumulator)
{
    return details::reduce_t<std::decay_t<Seed>, std::decay_t<Accumulator>>{std::forward<Seed>(seed), std::forward<Accumulator>(accumulator)};
}

/**
 * @brief Apply a function to each item emitted by an Observable, sequentially, and emit the final value
 *
 * @marble reduce
 {
     source observable                : +--1-2-3-|
     operator "reduce: (s,x)=>s+x"      : +-------6|
 }
 *
 * @details There is no initial value for seed, so, first value would be used as seed value and forwarded as is.
 * 
 * @param initial_value initial value for seed
 * @param accumulator function which accepts seed value and new value from observable and return new value of seed. Can accept seed by move-reference.
 *
 * @warning #include <rpp/operators/reduce.hpp>
 *
 * @par Example
 * @snippet reduce.cpp reduce_no_seed
 *
 * @ingroup aggregate_operators
 * @see https://reactivex.io/documentation/operators/reduce.html
 */
template<typename Accumulator>
auto reduce(Accumulator&& accumulator)
{
    return details::reduce_no_seed_t<std::decay_t<Accumulator>>{std::forward<Accumulator>(accumulator)};
}
}