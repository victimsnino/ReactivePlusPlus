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

#include <rpp/memory_model.hpp>
#include <rpp/schedulers/constraints.hpp>
#include <rpp/schedulers/fwd.hpp>
#include <rpp/utils/function_traits.hpp>
#include <rpp/subscribers/type_traits.hpp>
#include <rpp/utils/operator_declaration.hpp>

#include <rpp/observables/fwd.hpp>

#include <ranges>

namespace rpp::details
{
struct create_tag;
struct empty_tag;
struct never_tag;
struct error_tag;
struct just_tag;
struct from_tag;
} // rpp::observable::details

namespace rpp::observable
{
//**************************** CREATE ****************//
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe) requires rpp::details::is_header_included<rpp::details::create_tag, Type, OnSubscribeFn>;

template<utils::is_callable OnSubscribeFn, constraint::decayed_type Type = utils::extract_subscriber_type_t<utils::function_argument_t<OnSubscribeFn>>>
    requires constraint::on_subscribe_fn<OnSubscribeFn, Type>
auto create(OnSubscribeFn&& on_subscribe) requires rpp::details::is_header_included<rpp::details::create_tag, Type, OnSubscribeFn>;

//**************************** EMPTY *****************//
template<constraint::decayed_type Type>
auto empty() requires rpp::details::is_header_included<rpp::details::empty_tag, Type>;

//**************************** NEVER *****************//
template<constraint::decayed_type Type>
auto never() requires rpp::details::is_header_included<rpp::details::never_tag, Type>;

//**************************** ERROR *****************//
template<constraint::decayed_type Type>
auto error(const std::exception_ptr& err) requires rpp::details::is_header_included<rpp::details::error_tag, Type>;

//************************** FROM ***********************//
template<memory_model memory_model = memory_model::use_stack, typename T, typename ...Ts>
auto just(const schedulers::constraint::scheduler auto& scheduler, T&& item, Ts&& ...items) requires (rpp::details::is_header_included<rpp::details::just_tag, T, Ts...> && (constraint::decayed_same_as<T, Ts> && ...));

template<memory_model memory_model = memory_model::use_stack, typename T, typename ...Ts>
auto just(T&& item, Ts&& ...items) requires (rpp::details::is_header_included<rpp::details::just_tag, T, Ts...> && (constraint::decayed_same_as<T, Ts> && ...));

template<memory_model memory_model= memory_model::use_stack, schedulers::constraint::scheduler TScheduler =
    schedulers::immediate>
auto from_iterable(std::ranges::range auto&& iterable, const TScheduler& scheduler = TScheduler{}) requires rpp::details::is_header_included <rpp::details::from_tag, TScheduler > ;

template<memory_model memory_model = memory_model::use_stack>
auto from_callable(std::invocable<> auto&& callable) requires rpp::details::is_header_included<rpp::details::from_tag, decltype(callable)>;
} // namespace rpp::observable

namespace rpp
{
namespace source = observable;
} // namespace rpp
