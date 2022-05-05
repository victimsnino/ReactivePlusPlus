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
 * \brief This file contains implementation of `create` function to create rpp::specific_observable with OnSubscribe callback
 *
 * Examples:
 * \snippet create.cpp create
 * \snippet create.cpp create with capture
 * \snippet create.cpp create type deduction
 *
 * \see https://reactivex.io/documentation/operators/create.html
 **/

#include <rpp/sources/fwd.hpp>
#include <rpp/observables/specific_observable.hpp>
#include <type_traits>

namespace rpp::observable
{
/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable with passed action as OnSubscribe
 * \tparam Type manually specified type of value provided by this observable
 * \tparam OnSubscribeFn action called after subscription on this observable
 * \return rpp::specific_observable with passed action
 *
 * Examples:
 * \snippet create.cpp create
 * \snippet create.cpp create with capture
 * \snippet create.cpp create type deduction
 *
 * \see https://reactivex.io/documentation/operators/create.html
 */
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe)
{
    return specific_observable<Type, std::decay_t<OnSubscribeFn>>{std::forward<OnSubscribeFn>(on_subscribe)};
}

/**
 * \ingroup observables
 * \brief Creates specific_observable with passed action as OnSubscribe and deduce type of observable by this function
 * \tparam OnSubscribeFn action called after subscription on this observable
 * \tparam Type type of values deduced by argument of function
 * \return specific_observable with passed action
 *
 * Examples:
 * \snippet create.cpp create
 * \snippet create.cpp create with capture
 * \snippet create.cpp create type deduction
 *
 * \see https://reactivex.io/documentation/operators/create.html
 */
template<utils::is_callable OnSubscribeFn, constraint::decayed_type Type>
auto create(OnSubscribeFn&& on_subscribe)
{
    return create<Type>(std::forward<OnSubscribeFn>(on_subscribe));
}
} // namespace rpp::observable