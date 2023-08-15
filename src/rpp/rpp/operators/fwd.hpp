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

namespace rpp::operators
{
auto as_blocking();

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
auto merge_with(TObservable&& observable, TObservables&& ...observables);
auto merge();

auto repeat(size_t count);
auto repeat();

template<typename InitialValue, typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<std::decay_t<InitialValue>, std::invoke_result_t<Fn, std::decay_t<InitialValue>&&, rpp::utils::convertible_to_any>>)
auto scan(InitialValue&& initial_value, Fn&& accumulator);

template<typename Fn>
auto scan(Fn&& accumulator);

auto skip(size_t count);

template<rpp::schedulers::constraint::scheduler Scheduler>
auto subscribe_on(Scheduler&& scheduler);

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<bool, std::invoke_result_t<Fn, rpp::utils::convertible_to_any>>)
auto take_while(Fn&& predicate);

auto take(size_t count);

template<typename TSelector, rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires(!utils::is_not_template_callable<TSelector> ||
             std::invocable<TSelector, rpp::utils::convertible_to_any, utils::extract_observable_type_t<TObservable>, utils::extract_observable_type_t<TObservables>...>)
auto with_latest_from(TSelector&& selector, TObservable&& observable, TObservables&&... observables);
} // namespace rpp::operators

namespace rpp
{
    namespace ops = operators;
}
