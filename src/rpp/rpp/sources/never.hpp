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

#include <rpp/sources/create.hpp>
#include <rpp/sources/fwd.hpp>
#include <rpp/utils/constraints.hpp>


namespace rpp::observable
{
/**
 * \brief Creates rpp::specific_observable that emits no items and does not terminate
 * 
 * \marble never
   {
       operator "never": +>
   }
 * \tparam Type type of value to specify observable
 *
 * \ingroup creational_operators
 * \see https://reactivex.io/documentation/operators/empty-never-throw.html
 */
template<constraint::decayed_type Type>
auto never()
{
    return create<Type>([](const auto&) { });
}
} // namespace rpp::observable
