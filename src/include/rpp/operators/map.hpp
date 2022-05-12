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

#include <rpp/observables/constraints.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/observables/type_traits.hpp>
#include <rpp/operators/fwd/map.hpp>
#include <rpp/utils/utilities.hpp>
#include <utility>

IMPLEMENTATION_FILE(map_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, std::invocable<Type> Callable>
auto map_impl(Callable&& callable)
{
    return [callable = std::forward<Callable>(callable)](auto&& value, const constraint::subscriber_of_type<std::invoke_result_t<Callable, Type>> auto& subscriber)
    {
        subscriber.on_next(callable(utils::as_const(std::forward<decltype(value)>(value))));
    };
}
} // namespace rpp::details
