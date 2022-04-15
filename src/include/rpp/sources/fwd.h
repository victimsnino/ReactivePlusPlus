//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/memory_model.h>
#include <rpp/schedulers/constraints.h>
#include <rpp/schedulers/fwd.h>
#include <rpp/utils/function_traits.h>
#include <rpp/subscribers/type_traits.h>

#include <rpp/observables/fwd.h>

#include <ranges>

namespace rpp::observable
{
//**************************** CREATE ****************//
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe);

template<utils::is_callable OnSubscribeFn, constraint::decayed_type Type = utils::extract_subscriber_type_t<utils::function_argument_t<OnSubscribeFn>>>
auto create(OnSubscribeFn&& on_subscribe);

//**************************** EMPTY *****************//
template<constraint::decayed_type Type>
auto empty();

//**************************** NEVER *****************//
template<constraint::decayed_type Type>
auto never();

//**************************** ERROR *****************//
template<constraint::decayed_type Type>
auto error(const std::exception_ptr& err);

//*************************** JUST ********************//
template<memory_model memory_model = memory_model::use_stack, typename T, typename ...Ts>
auto just(const schedulers::constraint::scheduler auto& scheduler, T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...);

template<memory_model memory_model = memory_model::use_stack, typename T, typename ...Ts>
auto just(T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...);

//************************** FROM ***********************//
template<memory_model memory_model= memory_model::use_stack, schedulers::constraint::scheduler TScheduler = rpp::schedulers::immediate>
auto from(std::ranges::range auto&& iterable, const TScheduler& scheduler = TScheduler{});
} // namespace rpp::observable

namespace rpp
{
namespace source = observable;
} // namespace rpp
