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

#include <rpp/observers/fwd.hpp>
#include <rpp/observables/fwd.hpp>
#include <rpp/schedulers/fwd.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/utils.hpp>
#include <rpp/utils/function_traits.hpp>
#include <rpp/memory_model.hpp>
#include <exception>

namespace rpp::constraint
{
template<typename S, typename T>
concept on_subscribe = requires(const S& strategy, dynamic_observer<T>&& observer) {
{
    strategy(std::move(observer))} -> std::same_as<void>;
};
}

namespace rpp::source
{
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
auto create(OnSubscribe&& on_subscribe);

template<utils::is_not_template_callable OnSubscribe, constraint::decayed_type Type = rpp::utils::extract_observer_type_t<rpp::utils::decayed_function_argument_t<OnSubscribe>>>
auto create(OnSubscribe&& on_subscribe);

template<constraint::memory_model memory_model= memory_model::use_stack, constraint::iterable Iterable, schedulers::constraint::scheduler TScheduler = schedulers::current_thread>
auto from_iterable(Iterable&& iterable, const TScheduler& scheduler = TScheduler{});

template<constraint::memory_model memory_model = memory_model::use_stack, typename T, typename ...Ts>
auto just(T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...);

template<constraint::memory_model memory_model = memory_model::use_stack, schedulers::constraint::scheduler TScheduler, typename T, typename ...Ts>
auto just(const TScheduler& scheduler, T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...);

template<constraint::memory_model memory_model = memory_model::use_stack, std::invocable<> Callable>
auto from_callable(Callable&& callable);

template<constraint::memory_model memory_model = memory_model::use_stack, rpp::constraint::observable TObservable, rpp::constraint::observable ...TObservables>
    requires (std::same_as<rpp::utils::extract_observable_type_t<TObservable>, rpp::utils::extract_observable_type_t<TObservables>> && ...)
auto concat(TObservable&& obs, TObservables&&...others);

template<constraint::memory_model memory_model = memory_model::use_stack, constraint::iterable Iterable>
    requires constraint::observable<utils::iterable_value_t<Iterable>>
auto concat(Iterable&& iterable);

template<constraint::decayed_type Type>
auto never();

template<constraint::decayed_type Type>
auto error(std::exception_ptr err);

template<constraint::decayed_type Type>
auto empty();
} // namespace rpp::source