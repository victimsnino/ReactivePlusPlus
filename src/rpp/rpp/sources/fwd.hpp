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
#include <rpp/schedulers/fwd.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/memory_model.hpp>

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

template<typename OnSubscribe, constraint::decayed_type Type = rpp::utils::extract_observer_type_t<rpp::utils::decayed_function_argument_t<OnSubscribe>>>
auto create(OnSubscribe&& on_subscribe);

template<constraint::memory_model memory_model= memory_model::use_stack, schedulers::constraint::scheduler TScheduler = schedulers::current_thread>
auto from_iterable(constraint::iterable auto&& iterable, const TScheduler& scheduler = TScheduler{});

template<constraint::memory_model memory_model = memory_model::use_stack, typename T, typename ...Ts>
auto just(T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...);

template<constraint::memory_model memory_model = memory_model::use_stack, typename T, typename ...Ts>
auto just(const schedulers::constraint::scheduler auto& scheduler, T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...);
} // namespace rpp::source