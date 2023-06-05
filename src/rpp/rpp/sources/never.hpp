//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/sources/fwd.hpp>
#include <rpp/observables/observable.hpp>

namespace rpp::details
{
struct never_strategy
{
    static void subscribe(const auto&){}
};
}

namespace rpp
{
template<constraint::decayed_type Type>
using never_observable = observable<Type, details::never_strategy>;
}

namespace rpp::source
{
/**
 * @brief Creates rpp::observable that emits no items and does not terminate
 *
 * @marble never
   {
       operator "never": +>
   }
 * @tparam Type type of value to specify observable
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/empty-never-throw.html
 */
template<constraint::decayed_type Type>
auto never()
{
    return never_observable<Type>{};
}
}