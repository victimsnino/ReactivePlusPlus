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

/**
 * \file
 * \brief This file contains implementation of `error` functions to create rpp::specific_observable
 *
 * \see https://reactivex.io/documentation/operators/empty-never-throw.html
 **/

#include <rpp/sources/create.h>
#include <rpp/sources/fwd.h>
#include <rpp/utils/constraints.h>

#include <exception>

namespace rpp::observable
{
/**
  * \ingroup observables
  * \brief Creates rpp::specific_observable that emits no items and terminates with an error
  * \tparam Type type of value to specify observable
  * \param err exception ptr to be sent to subscriber
  *
  * \see https://reactivex.io/documentation/operators/empty-never-throw.html
  */
template<constraint::decayed_type Type>
auto error(const std::exception_ptr& err)
{
    return create<Type>([err](const auto& sub)
    {
        sub.on_error(err);
    });
}
} // namespace rpp::observable
