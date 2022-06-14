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
#include <rpp/operators/fwd/map.hpp>
#include <rpp/utils/utilities.hpp>
#include <utility>

IMPLEMENTATION_FILE(map_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, std::invocable<Type> Callable>
struct map_impl
{
    Callable callable;

    template<typename TVal, constraint::subscriber_of_type<std::invoke_result_t<Callable, Type>> TSub>
    void operator()(TVal&& value, const TSub& subscriber) const
    {
        subscriber.on_next(callable(utils::as_const(std::forward<TVal>(value))));
    };
};
} // namespace rpp::details
