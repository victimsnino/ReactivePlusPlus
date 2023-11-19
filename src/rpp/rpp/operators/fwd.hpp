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

#include <rpp/observables/fwd.hpp>
#include <rpp/schedulers/fwd.hpp>
#include <rpp/subjects/fwd.hpp>

#include <rpp/utils/constraints.hpp>
#include <rpp/utils/utils.hpp>
#include <rpp/memory_model.hpp>

namespace rpp::operators
{
auto as_blocking();

auto buffer(size_t count);

template<typename TSelector, rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires (!rpp::constraint::observable<TSelector> && (!utils::is_not_template_callable<TSelector> || std::invocable<TSelector, rpp::utils::convertible_to_any, utils::extract_observable_type_t<TObservable>, utils::extract_observable_type_t<TObservables>...>))
auto combine_latest(TSelector&& selector, TObservable&& observable, TObservables&&... observables);

template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
auto combine_latest(TObservable&& observable, TObservables&&... observables);

template<rpp::schedulers::constraint::scheduler Scheduler>
auto debounce(rpp::schedulers::duration period, Scheduler&& scheduler);

template<rpp::schedulers::constraint::scheduler Scheduler>
auto delay(rpp::schedulers::duration delay_duration, Scheduler&& scheduler);

template<typename EqualityFn = rpp::utils::equal_to>
    requires (!utils::is_not_template_callable<EqualityFn> || std::same_as<bool, std::invoke_result_t<EqualityFn, rpp::utils::convertible_to_any, rpp::utils::convertible_to_any>>)
auto distinct_until_changed(EqualityFn&& equality_fn = {});

auto first();

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<bool, std::invoke_result_t<Fn, rpp::utils::convertible_to_any>>)
auto filter(Fn&& predicate);

template<typename KeySelector,
         typename ValueSelector = std::identity,
         typename KeyComparator = rpp::utils::less>
    requires
    (
        (!utils::is_not_template_callable<KeySelector> || !std::same_as<void, std::invoke_result_t<KeySelector, rpp::utils::convertible_to_any>>) &&
        (!utils::is_not_template_callable<ValueSelector> || !std::same_as<void, std::invoke_result_t<ValueSelector, rpp::utils::convertible_to_any>>) &&
        (!utils::is_not_template_callable<KeyComparator> || std::strict_weak_order<KeyComparator, rpp::utils::convertible_to_any, rpp::utils::convertible_to_any>)
    )
auto group_by(KeySelector&& key_selector, ValueSelector&& value_selector = {}, KeyComparator&& comparator = {});

auto last();

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || !std::same_as<void, std::invoke_result_t<Fn, rpp::utils::convertible_to_any>>)
auto map(Fn&& callable);

template<rpp::constraint::subject Subject>
auto multicast(Subject&& subject);

template<template<typename> typename Subject = rpp::subjects::publish_subject>
auto multicast();

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || rpp::constraint::observable<std::invoke_result_t<Fn, rpp::utils::convertible_to_any>>)
auto flat_map(Fn&& callable);

template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires constraint::observables_of_same_type<std::decay_t<TObservable>, std::decay_t<TObservables>...>
auto merge_with(TObservable&& observable, TObservables&&... observables);
auto merge();

template<rpp::schedulers::constraint::scheduler Scheduler>
auto observe_on(Scheduler&& scheduler, rpp::schedulers::duration delay_duration = {});

auto publish();

auto ref_count();

template<rpp::schedulers::constraint::scheduler Scheduler = rpp::schedulers::defaults::iteration_scheduler>
auto repeat(size_t count, const Scheduler& scheduler = {});

template<rpp::schedulers::constraint::scheduler Scheduler = rpp::schedulers::defaults::iteration_scheduler>
auto repeat(const Scheduler& scheduler = {});

template<typename InitialValue, typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<std::decay_t<InitialValue>, std::invoke_result_t<Fn, std::decay_t<InitialValue>&&, rpp::utils::convertible_to_any>>)
auto scan(InitialValue&& initial_value, Fn&& accumulator);

template<typename Fn>
auto scan(Fn&& accumulator);

auto skip(size_t count);

template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires constraint::observables_of_same_type<std::decay_t<TObservable>, std::decay_t<TObservables>...>
auto start_with(TObservable&& observable, TObservables&&... observables);

template<constraint::memory_model MemoryModel = memory_model::use_stack, typename T, typename ...Ts>
    requires ((rpp::constraint::decayed_same_as<T, Ts> && ...) && !(rpp::constraint::observable<T> || (rpp::constraint::observable<Ts> || ...)))
auto start_with(T&& v, Ts&&... vals);

template<constraint::memory_model MemoryModel = memory_model::use_stack, rpp::schedulers::constraint::scheduler TScheduler, typename T, typename ...Ts>
    requires ((rpp::constraint::decayed_same_as<T, Ts> && ...) && !(rpp::constraint::observable<T> || (rpp::constraint::observable<Ts> || ...)))
auto start_with(const TScheduler& scheduler, T&& v, Ts&&... vals);

template<constraint::memory_model MemoryModel = memory_model::use_stack, typename T, typename ...Ts>
    requires (rpp::constraint::decayed_same_as<T, Ts> && ...)
auto start_with_values(T&& v, Ts&&... vals);

template<constraint::memory_model MemoryModel = memory_model::use_stack, rpp::schedulers::constraint::scheduler TScheduler, typename T, typename ...Ts>
    requires (rpp::constraint::decayed_same_as<T, Ts> && ...)
auto start_with_values(const TScheduler& scheduler, T&& v, Ts&&... vals);

template<rpp::schedulers::constraint::scheduler Scheduler>
auto subscribe_on(Scheduler&& scheduler);

auto take(size_t count);

auto take_last(size_t count);

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<bool, std::invoke_result_t<Fn, rpp::utils::convertible_to_any>>)
auto take_while(Fn&& predicate);

template<rpp::constraint::observable TObservable>
auto take_until(TObservable&& until_observable);

template<rpp::schedulers::constraint::scheduler Scheduler = rpp::schedulers::immediate>
auto throttle(rpp::schedulers::duration period);

template<typename TSelector, rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires(!rpp::constraint::observable<TSelector> && (!utils::is_not_template_callable<TSelector> || std::invocable<TSelector, rpp::utils::convertible_to_any, utils::extract_observable_type_t<TObservable>, utils::extract_observable_type_t<TObservables>...>))
auto with_latest_from(TSelector&& selector, TObservable&& observable, TObservables&&... observables);

template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
auto with_latest_from(TObservable&& observable, TObservables&&... observables);

auto window(size_t count);
} // namespace rpp::operators

namespace rpp
{
namespace ops = operators;
}
