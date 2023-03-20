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

#include <concepts>
#include <rpp/observers/fwd.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/memory_model.hpp>
#include <rpp/utils/operator_declaration.hpp>
#include <rpp/utils/function_traits.hpp>

namespace rpp::details
{
struct create_tag{};
struct from_tag{};
}

namespace rpp::source
{
template<constraint::decayed_type Type, std::invocable<rpp::interface_observer<Type>> OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe) requires rpp::details::is_header_included<rpp::details::create_tag, Type, OnSubscribeFn>;

template<utils::is_callable OnSubscribeFn, constraint::decayed_type Type = utils::extract_observer_type_t<utils::function_argument_t<OnSubscribeFn>>>
    requires std::invocable<OnSubscribeFn, rpp::interface_observer<Type>>
auto create(OnSubscribeFn&& on_subscribe) requires rpp::details::is_header_included<rpp::details::create_tag, Type, OnSubscribeFn>;

template<memory_model memory_model = memory_model::use_stack/*, schedulers::constraint::scheduler TScheduler = schedulers::trampoline*/>
auto from_iterable(constraint::iterable auto&& iterable/*, const TScheduler& scheduler = TScheduler{}*/)requires rpp::details::is_header_included<rpp::details::from_tag, decltype(iterable) /*TScheduler*/>;
} // namespace rpp::sopurce

