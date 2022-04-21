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
 * \brief This file contains implementation of `empty` function to create rpp::specific_observable
 *
 * \see https://reactivex.io/documentation/operators/empty-never-throw.hpptml
 **/

#include <rpp/sources/create.hpp>
#include <rpp/sources/fwd.hpp>
#include <rpp/utils/constraints.hpp>


namespace rpp::observable
{
/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable that emits no items but terminates normally
 * \tparam Type type of value to specify observable
 *
 * \see https://reactivex.io/documentation/operators/empty-never-throw.hpptml
 */
template<constraint::decayed_type Type>
auto empty()
{
    return create<Type>([](const auto& sub)
    {
        sub.on_completed();
    });
}
} // namespace rpp::observable
