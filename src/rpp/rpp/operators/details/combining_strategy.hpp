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
#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/operators/details/utils.hpp>
#include <rpp/schedulers/current_thread.hpp>

#include <memory>

namespace rpp::operators::details
{
template<rpp::constraint::observer Observer, rpp::constraint::decayed_type... Args>
class combining_disposable : public composite_disposable
{
public:
    explicit combining_disposable(Observer&& observer)
        : m_observer_with_mutex{std::move(observer)}
    {
    }

    pointer_under_lock<Observer> get_observer_under_lock() { return pointer_under_lock{m_observer_with_mutex}; }

    bool decrement_on_completed()
    {
        // just need atomicity, not guarding anything
        return m_on_completed_needed.fetch_sub(1, std::memory_order::seq_cst) == 1;
    }

private:
    value_with_mutex<Observer> m_observer_with_mutex{};

    std::atomic_size_t m_on_completed_needed{sizeof...(Args)};
};

template<typename TDisposable>
struct combining_observer_strategy
{
    std::shared_ptr<TDisposable> disposable{};

    void set_upstream(const rpp::disposable_wrapper& d) const
    {
        disposable->add(d);
    }

    bool is_disposed() const
    {
        return disposable->is_disposed();
    }

    void on_error(const std::exception_ptr& err) const
    {
        disposable->dispose();
        disposable->get_observer_under_lock()->on_error(err);
    }

    void on_completed() const
    {
        if (disposable->decrement_on_completed())
        {
            disposable->dispose();
            disposable->get_observer_under_lock()->on_completed();
        }
    }
};

template<template<typename...> typename TDisposable, template<auto, typename...> typename TStrategy, typename TSelector, rpp::constraint::observable... TObservables>
struct combining_operator_t
{
    RPP_NO_UNIQUE_ADDRESS rpp::utils::tuple<TObservables...> observables;
    RPP_NO_UNIQUE_ADDRESS TSelector                          selector;

    template<rpp::constraint::decayed_type T>
        requires std::invocable<TSelector, T, rpp::utils::extract_observable_type_t<TObservables>...>
    using result_value = std::invoke_result_t<TSelector, T, rpp::utils::extract_observable_type_t<TObservables>...>;

    template<rpp::details::observables::constraint::disposable_strategy Prev>
    using updated_disposable_strategy = rpp::details::observables::fixed_disposable_strategy_selector<1>;

    template<rpp::constraint::observer Observer, typename... Strategies>
    void subscribe(Observer&& observer, const observable_chain_strategy<Strategies...>& observable_strategy) const
    {
        // Need to take ownership over current_thread in case of inner-observables also using it
        auto drain_on_exit = rpp::schedulers::current_thread::own_queue_and_drain_finally_if_not_owned();
        observables.apply(&subscribe_impl<Observer, Strategies...>, std::forward<Observer>(observer), observable_strategy, selector);
    }

private:
    template<rpp::constraint::observer Observer, typename... Strategies>
    static void subscribe_impl(Observer&& observer, const observable_chain_strategy<Strategies...>& observable_strategy, const TSelector& selector, const TObservables&... observables)
    {
        using ExpectedValue = typename observable_chain_strategy<Strategies...>::value_type;
        using Disposable    = TDisposable<Observer, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>;

        const auto disposable = disposable_wrapper_impl<Disposable>::make(std::forward<Observer>(observer), selector);
        auto       locked     = disposable.lock();
        locked->get_observer_under_lock()->set_upstream(disposable.as_weak());
        subscribe<std::decay_t<ExpectedValue>>(locked, std::index_sequence_for<TObservables...>{}, observables...);

        observable_strategy.subscribe(rpp::observer<ExpectedValue, TStrategy<0, std::decay_t<Observer>, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>>{std::move(locked)});
    }

    template<typename ExpectedValue, rpp::constraint::observer Observer, size_t... I>
    static void subscribe(const std::shared_ptr<TDisposable<Observer, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>>& disposable, std::index_sequence<I...>, const TObservables&... observables)
    {
        (..., observables.subscribe(rpp::observer<rpp::utils::extract_observable_type_t<TObservables>, TStrategy<I + 1, Observer, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>>{disposable}));
    }
};
}