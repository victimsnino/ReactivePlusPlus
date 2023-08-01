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
#include <rpp/disposables/composite_disposable.hpp>

#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <rpp/utils/tuple.hpp>

namespace rpp::operators::details
{
template<rpp::constraint::observer TObserver>
class merge_disposable final : public composite_disposable
{
public:
    merge_disposable(TObserver&& observer) : m_observer(std::move(observer)) {}

    std::lock_guard<std::mutex> lock_guard() { return std::lock_guard<std::mutex>{m_mutex}; }

    void increment_on_completed() { m_on_completed_needed.fetch_add(1, std::memory_order_relaxed); }
    bool decrement_on_completed() { return m_on_completed_needed.fetch_sub(1, std::memory_order::acq_rel) == 1; }

    TObserver& get_observer() { return m_observer; }

private:
    TObserver          m_observer;
    std::mutex         m_mutex{};
    std::atomic_size_t m_on_completed_needed{1};
};

template<rpp::constraint::observer TObserver>
struct merge_observer_base_strategy
{
    merge_observer_base_strategy(std::shared_ptr<merge_disposable<TObserver>>&& disposable)
        : m_disposable{std::move(disposable)}
    {}

    merge_observer_base_strategy(const std::shared_ptr<merge_disposable<TObserver>>& disposable)
        : m_disposable{disposable}
    {}

    void set_upstream(const rpp::disposable_wrapper& d) const
    {
        m_disposable->add(d.get_original());
    }

    bool is_disposed() const
    {
        return m_disposable->is_disposed() || m_disposable->get_observer().is_disposed();
    }

    void on_error(const std::exception_ptr& err) const
    {
        m_disposable->dispose();

        auto lock = m_disposable->lock_guard();
        m_disposable->get_observer().on_error(err);
    }

    void on_completed() const
    {
        if (m_disposable->decrement_on_completed())
        {
            m_disposable->dispose();

            auto lock = m_disposable->lock_guard();
            m_disposable->get_observer().on_completed();
        }
    }

protected:
    std::shared_ptr<merge_disposable<TObserver>> m_disposable;
};

template<rpp::constraint::observer TObserver>
struct merge_observer_inner_strategy final : public merge_observer_base_strategy<TObserver>
{
    using merge_observer_base_strategy<TObserver>::merge_observer_base_strategy;

    template<typename T>
    void on_next(T&& v) const
    {
        auto lock = merge_observer_base_strategy<TObserver>::m_disposable->lock_guard();
        merge_observer_base_strategy<TObserver>::m_disposable->get_observer().on_next(std::forward<T>(v));
    }
};

template<rpp::constraint::observer TObserver>
class merge_observer_strategy final : public merge_observer_base_strategy<TObserver> 
{
public:
    explicit merge_observer_strategy(TObserver&& observer)
        : merge_observer_base_strategy<TObserver>{std::make_shared<merge_disposable<TObserver>>(std::move(observer))}
    {
        merge_observer_base_strategy<TObserver>::m_disposable->get_observer().set_upstream(disposable_wrapper::from_weak(merge_observer_base_strategy<TObserver>::m_disposable));
    }

    template<typename T>
    void on_next(T&& v) const
    {
        merge_observer_base_strategy<TObserver>::m_disposable->increment_on_completed();
        std::forward<T>(v).subscribe(rpp::observer<rpp::utils::extract_observer_type_t<TObserver>, merge_observer_inner_strategy<TObserver>>{merge_observer_inner_strategy<TObserver>{merge_observer_base_strategy<TObserver>::m_disposable}});
    }
};

struct merge_t
{
    template<rpp::constraint::decayed_type T>
        requires rpp::constraint::observable<T>
    using ResultValue = rpp::utils::extract_observable_type_t<T>;

    template<rpp::constraint::observer Observer, typename... Strategies>
    void subscribe(Observer&& observer, const observable_chain_strategy<Strategies...>& strategy) const
    {
        // Need to take ownership over current_thread in case of inner-observables also uses them
        auto drain_on_exit = rpp::schedulers::current_thread::own_queue_and_drain_finally_if_not_owned();

        using InnerObservable = typename observable_chain_strategy<Strategies...>::ValueType;

        strategy.subscribe(rpp::observer<InnerObservable, merge_observer_strategy<std::decay_t<Observer>>>{std::forward<Observer>(observer)});
    }

};

template<rpp::constraint::observable... TObservables>
struct merge_with_t
{
    RPP_NO_UNIQUE_ADDRESS rpp::utils::tuple<TObservables...> observables{};

    template<rpp::constraint::decayed_type T>
        requires (std::same_as<T, utils::extract_observable_type_t<TObservables>> && ...)
    using ResultValue = T;

    template<rpp::constraint::observer Observer, typename... Strategies>
    void subscribe(Observer&& observer, const observable_chain_strategy<Strategies...>& observable_strategy) const
    {
        merge_observer_strategy<std::decay_t<Observer>> strategy{std::forward<Observer>(observer)};

        // Need to take ownership over current_thread in case of inner-observables also uses them
        auto drain_on_exit = rpp::schedulers::current_thread::own_queue_and_drain_finally_if_not_owned();

        strategy.on_next(observable_strategy);
        observables.apply(&apply<std::decay_t<Observer>>, strategy);
        strategy.on_completed();
    }

private:
    template<rpp::constraint::observer Observer>
    static void apply(const merge_observer_strategy<Observer>& strategy, const TObservables& ...observables)
    {
        (strategy.on_next(observables), ...);
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
 * @warning During on subscribe operator takes ownership over rpp::schedulers::current_thread to allow mixing of underlying emissions
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
 * @warning During on subscribe operator takes ownership over rpp::schedulers::current_thread to allow mixing of underlying emissions
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
    return details::merge_with_t<std::decay_t<TObservable>, std::decay_t<TObservables>...>{rpp::utils::tuple{std::forward<TObservable>(observable), std::forward<TObservables>(observables)...}};
}
} // namespace rpp::operators
