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
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/utils.hpp>

namespace rpp::operators
{
auto as_blocking();

template<typename EqualityFn = utils::equal_to>
    requires (!utils::is_not_template_callable<EqualityFn> || std::same_as<bool, std::invoke_result_t<EqualityFn, utils::convertible_to_any, utils::convertible_to_any>>)
auto distinct_until_changed(EqualityFn&& equality_fn = {});

auto first();

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<bool, std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto filter(Fn&& predicate);

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || !std::same_as<void, std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto map(Fn&& callable);

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || rpp::constraint::observable<std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto flat_map(Fn&& callable);

template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires constraint::observables_of_same_type<std::decay_t<TObservable>, std::decay_t<TObservables>...>
auto merge_with(TObservable&& observable, TObservables&& ...observables);
auto merge();

auto repeat(size_t count);
auto repeat();

template<typename InitialValue, typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<std::decay_t<InitialValue>, std::invoke_result_t<Fn, std::decay_t<InitialValue>&&, utils::convertible_to_any>>)
auto scan(InitialValue&& initial_value, Fn&& accumulator);

template<typename Fn>
auto scan(Fn&& accumulator);

auto skip(size_t count);

template<rpp::schedulers::constraint::scheduler Scheduler>
auto subscribe_on(Scheduler&& scheduler);

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<bool, std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto take_while(Fn&& predicate);

auto take(size_t count);
}

namespace rpp
{
    namespace ops = operators;
}
