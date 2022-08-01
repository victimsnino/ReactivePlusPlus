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

#include <rpp/utils/constraints.hpp>

namespace rpp::details
{
struct observer_tag{};
} // namespace rpp::details

namespace rpp::constraint
{
template<typename Fn, typename Type> concept on_next_fn      = std::invocable<std::decay_t<Fn>, Type>;
template<typename Fn>                concept on_error_fn     = std::invocable<std::decay_t<Fn>, std::exception_ptr>;
template<typename Fn>                concept on_completed_fn = std::invocable<std::decay_t<Fn>>;
} // namespace rpp::constraint

namespace rpp
{

template<constraint::decayed_type T>
struct typed_observer : public details::observer_tag {};

template<constraint::decayed_type Type>
class dynamic_observer;

template<constraint::decayed_type T,
         constraint::on_next_fn<T>   OnNext,
         constraint::on_error_fn     OnError,
         constraint::on_completed_fn OnCompleted>
class specific_observer;
} // namespace rpp