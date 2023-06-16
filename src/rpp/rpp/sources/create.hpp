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
#include <rpp/operators/details/strategy.hpp>

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
struct create_strategy
{
    RPP_NO_UNIQUE_ADDRESS OnSubscribe subscribe;
};
}

namespace rpp
{
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
RPP_OPERATOR_OBSERVBLE_IMPL(create_observable, observable, observable, Type, details::create_strategy<Type, OnSubscribe>);
}

namespace rpp::source
{
/**
 * @brief Construct observable specialized with passed callback function. Most easiesest way to construct observable "on the fly" via lambda and etc.
 *
 * @marble create
   {
       operator "create:  on_next(1), on_next(3), on_completed()": +--1--3--|
   }
 *
 * @warning Be sure, that your callback doesn't violates observable rule - observable must to emit emissions in serial way
 *
 * @tparam Type is type of values observable would emit
 * @tparam OnSubscribe is callback function to implement core logic of observable
 *
 * @par Examples:
 * @snippet create.cpp create
 * @snippet create.cpp create with capture
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/create.html
 */
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
auto create(OnSubscribe&& on_subscribe)
{
    return create_observable<Type, std::decay_t<OnSubscribe>>{std::forward<OnSubscribe>(on_subscribe)};
}

/**
 * @brief Construct observable specialized with passed callback function. Most easiesest way to construct observable "on the fly" via lambda and etc.
 *
 * @marble create
   {
       operator "create:  on_next(1), on_next(3), on_completed()": +--1--3--|
   }
 *
 * @warning Be sure, that your callback doesn't violates observable rules:
 * 1) observable must to emit emissions in serial way
 * 2) observable must not to call any callbacks after termination events - on_error/on_completed
 *
 * @tparam Type is type of values observable would emit
 * @tparam OnSubscribe is callback function to implement core logic of observable
 *
 * @par Examples:
 * @snippet create.cpp create
 * @snippet create.cpp create with capture
 * @snippet create.cpp create type deduction
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/create.html
 */
template<typename OnSubscribe, constraint::decayed_type Type>
auto create(OnSubscribe&& on_subscribe)
{
    return create<Type>(std::forward<OnSubscribe>(on_subscribe));
}
}