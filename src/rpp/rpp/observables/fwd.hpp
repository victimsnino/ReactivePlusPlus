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

#include <rpp/subscribers/fwd.hpp>   // on_subscribe_fn
#include <rpp/utils/constraints.hpp> // decayed type

namespace rpp::details
{
struct observable_tag;
struct dynamic_observable_tag;
} // namespace rpp::details

namespace rpp::constraint
{
template<typename Fn, typename T> concept on_subscribe_fn = std::invocable<std::decay_t<Fn>, dynamic_subscriber<T>>;
} // namespace rpp::constraint

namespace rpp
{
template<constraint::decayed_type Type>
struct virtual_observable;

template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
class specific_observable;

template<constraint::decayed_type Type>
class dynamic_observable;

template<constraint::decayed_type KeyType,
         constraint::decayed_type Type,
         constraint::on_subscribe_fn<Type> OnSubscribeFn>
class grouped_observable;
} // namespace rpp
