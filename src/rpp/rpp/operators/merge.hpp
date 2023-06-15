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
#include <rpp/schedulers/current_thread.hpp>

#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <tuple>

namespace rpp::operators::details
{
template<rpp::constraint::observable Observable>
class merge_observable_strategy;
}

namespace rpp
{
template<rpp::constraint::observable TObservable>
RPP_OPERATOR_OBSERVBLE_IMPL(merge_observable, rpp::observable, observable, rpp::utils::extract_observable_type_t<rpp::utils::extract_observable_type_t<TObservable>>, operators::details::merge_observable_strategy<TObservable>);
}

namespace rpp::operators::details
{
template<typename Lock>
class merge_disposable final : public base_disposable
{
public:
    std::lock_guard<Lock> lock_guard() { return std::lock_guard<Lock>{m_lock}; }

    void increment_on_completed() { m_on_completed_needed.fetch_add(1, std::memory_order_relaxed); }
    bool decrement_on_completed() { return m_on_completed_needed.fetch_sub(1, std::memory_order::acq_rel) == 1; }

private:
    Lock               m_lock{};
    std::atomic_size_t m_on_completed_needed{};
};

struct merge_observer_inner_strategy
{
    std::shared_ptr<merge_disposable<std::mutex>> disposable;

    static constexpr empty_on_subscribe on_subscribe{};

    void set_upstream(const rpp::constraint::observer auto&, const rpp::disposable_wrapper& d) const
    {
        disposable->add(d.get_original());
    }

    bool is_disposed() const
    {
        return disposable->is_disposed();
    }

    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        auto lock = disposable->lock_guard();
        obs.on_next(std::forward<T>(v));
    }

    void on_error(const rpp::constraint::observer auto & obs, const std::exception_ptr& err) const
    {
        disposable->dispose();

        auto lock = disposable->lock_guard();
        obs.on_error(err);
    }

    void on_completed(const rpp::constraint::observer auto& obs) const
    {
        if (disposable->decrement_on_completed())
        {
            disposable->dispose();

            auto lock = disposable->lock_guard();
            obs.on_completed();
        }
    }
};

template<rpp::constraint::decayed_type Value>
struct merge_observer_strategy
{
    std::shared_ptr<merge_disposable<std::mutex>> disposable = std::make_shared<merge_disposable<std::mutex>>();

    void on_subscribe(rpp::dynamic_observer<Value>& obs) const
    {
        disposable->increment_on_completed();
        obs.set_upstream(disposable_wrapper{disposable});
    }

    void set_upstream(const rpp::constraint::observer auto&, const rpp::disposable_wrapper& d) const
    {
        disposable->add(d.get_original());
    }

    bool is_disposed() const
    {
        return disposable->is_disposed();
    }

    template<typename T>
    void on_next(rpp::dynamic_observer<Value> obs, T&& v) const
    {
        disposable->increment_on_completed();
        std::forward<T>(v).subscribe(rpp::observer<Value, operator_strategy_base<rpp::dynamic_observer<Value>, merge_observer_inner_strategy>>{std::move(obs), disposable});
    }

    void on_error(const rpp::constraint::observer auto & obs, const std::exception_ptr& err) const
    {
        disposable->dispose();

        auto lock = disposable->lock_guard();
        obs.on_error(err);
    }

    void on_completed(const rpp::constraint::observer auto& obs) const
    {
        if (disposable->decrement_on_completed())
        {
            disposable->dispose();

            auto lock = disposable->lock_guard();
            obs.on_completed();
        }
    }
};


template<rpp::constraint::observable Observable>
class merge_observable_strategy
{
    using InnerObservable = rpp::utils::extract_observable_type_t<Observable>;
    using Value = rpp::utils::extract_observable_type_t<InnerObservable>;
public:
    merge_observable_strategy(const Observable& observable)
        : m_observable{observable} {}

    merge_observable_strategy(Observable&& observable)
        : m_observable{std::move(observable)} {}

    template<rpp::constraint::observer_strategy<Value> ObserverStrategy>
    void subscribe(rpp::observer<Value, ObserverStrategy>&& obs) const
    {
        // Need to take ownership over current_thread in case of inner-observables also uses them
        auto drain_on_exit = rpp::schedulers::current_thread::own_queue_and_drain_finally_if_not_owned();

        m_observable.subscribe(rpp::observer<InnerObservable, operator_strategy_base<rpp::dynamic_observer<Value>, merge_observer_strategy<Value>>>{std::move(obs).as_dynamic()});
    }

private:
    RPP_NO_UNIQUE_ADDRESS Observable m_observable;
};

struct merge_t
{
    template<rpp::constraint::observable TObservable>
        requires rpp::constraint::observable<rpp::utils::extract_observable_type_t<TObservable>>
    auto operator()(TObservable&& observable) const
    {
        return merge_observable<std::decay_t<TObservable>>{std::forward<TObservable>(observable)};
    }
};


template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires rpp::constraint::observables_of_same_type<std::decay_t<TObservable>, std::decay_t<TObservables>...>
class merge_with_observable_strategy
{
    using Value = rpp::utils::extract_observable_type_t<TObservable>;
public:
    merge_with_observable_strategy(const std::tuple<TObservable, TObservables...>& observable)
        : m_observables{observable} {}

    merge_with_observable_strategy(std::tuple<TObservable, TObservables...>&& observable)
        : m_observables{std::move(observable)} {}

    template<rpp::constraint::observer_strategy<Value> ObserverStrategy>
    void subscribe(rpp::observer<Value, ObserverStrategy>&& obs) const
    {
        auto obs_as_dynamic = std::move(obs).as_dynamic();

        merge_observer_strategy<Value> strategy{};

        // Need to take ownership over current_thread in case of inner-observables also uses them
        auto drain_on_exit = rpp::schedulers::current_thread::own_queue_and_drain_finally_if_not_owned();

        strategy.on_subscribe(obs_as_dynamic);
        std::apply([&](const auto&... observables) { (strategy.on_next(obs_as_dynamic, observables), ...); }, m_observables);
        strategy.on_completed(obs_as_dynamic);
    }

private:
    std::tuple<TObservable, TObservables...> m_observables{};
};

template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
using merge_with_observable = rpp::observable<rpp::utils::extract_observable_type_t<TObservable>, merge_with_observable_strategy<TObservable, TObservables...>>;

template<rpp::constraint::observable... TObservables>
struct merge_with_t
{
    std::tuple<TObservables...> observables{};

    template<rpp::constraint::observable TObservable>
        requires rpp::constraint::observables_of_same_type<std::decay_t<TObservable>, std::decay_t<TObservables>...>
    auto operator()(TObservable&& observable) const &
    {
        return std::apply(
            [&observable](const TObservables&... observables)
            {
                return merge_with_observable<std::decay_t<TObservable>, TObservables...>{std::tuple{std::forward<TObservable>(observable), observables...}};
            },
            observables);
    }

    template<rpp::constraint::observable TObservable>
        requires rpp::constraint::observables_of_same_type<std::decay_t<TObservable>, std::decay_t<TObservables>...>
    auto operator()(TObservable&& observable) &&
    {
        return std::apply(
            [&observable](TObservables&&... observables)
            {
                return merge_with_observable<std::decay_t<TObservable>, TObservables...>{std::tuple{std::forward<TObservable>(observable), std::move(observables)...}};
            },
            std::move(observables));
    }
};
}

namespace rpp::operators
{
/**
 * @brief Converts observable of observables of items into observable of items via merging emissions.
 *
 * @warning According to observable contract (https://reactivex.io/documentation/contract.html) emissions from any observable should be serialized, so, resulting observable uses mutex to satisfy this requirement
 *
 * @warning During on subscribe operator takes ownership over rpp::schdulers::current_thread to allow mixing of underlying emissions
 *
 * @marble merge
     {
         source observable                :
         {
             +--1-2-3-|
             .....+4--6-|
         }
         operator "merge" : +--1-243-6-|
     }
 *
 * @details Actually it subscribes on each observable from emissions. Resulting observables completes when ALL observables completes
 *
 * @par Performance notes:
 * - 2 heap allocation (1 for state, 1 to convert observer to dynamic_observer)
 * - Acquiring mutex during all observer's calls
 *
 * @warning #include <rpp/operators/merge.hpp>
 *
 * @par Example:
 * @snippet merge.cpp merge
 *
 * @ingroup combining_operators
 * @see https://reactivex.io/documentation/operators/merge.html
 */
inline auto merge()
{
    return details::merge_t{};
}

/**
 * @brief Combines submissions from current observable with other observables into one
 *
 * @warning According to observable contract (https://reactivex.io/documentation/contract.html) emissions from any observable should be serialized, so, resulting observable uses mutex to satisfy this requirement
 *
 * @warning During on subscribe operator takes ownership over rpp::schdulers::current_thread to allow mixing of underlying emissions
 *
 * @marble merge_with
     {
         source original_observable: +--1-2-3-|
         source second: +-----4--6-|
         operator "merge_with" : +--1-243-6-|
     }
 *
 * @details Actually it subscribes on each observable. Resulting observables completes when ALL observables completes
 *
 * @par Performance notes:
 * - 2 heap allocation (1 for state, 1 to convert observer to dynamic_observer)
 * - Acquiring mutex during all observer's calls
 *
 * @param observables are observables whose emissions would be merged with current observable
 * @warning #include <rpp/operators/merge.hpp>
 *
 * @par Example:
 * @snippet merge.cpp merge_with
 *
 * @ingroup combining_operators
 * @see https://reactivex.io/documentation/operators/merge.html
 */
template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires constraint::observables_of_same_type<std::decay_t<TObservable>, std::decay_t<TObservables>...>
auto merge_with(TObservable&& observable, TObservables&& ...observables)
{
    return details::merge_with_t<std::decay_t<TObservable>, std::decay_t<TObservables>...>{std::tuple{std::forward<TObservable>(observable), std::forward<TObservables>(observables)...}};
}
} // namespace rpp::operators