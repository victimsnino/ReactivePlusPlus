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

#include <rpp/observers/fwd.hpp>
#include <rpp/utils/constraints.hpp>

namespace rpp::constraint
{
template<typename S, typename T>
concept on_subscribe = requires(const S& strategy, dynamic_observer<T>&& observer) {
{
    strategy(std::move(observer))} -> std::same_as<void>;
};
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
 * @warning Be sure, that your callback doesn't violates observable rules:
 * 1) observable must to emit emissions in serial way
 * 2) observable must not to call any callbacks after termination events - on_error/on_completed
 *
 * @tparam Type is type of values observable would emit
 * @tparam OnSubscribe is callback function to implement core logic of observable
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/create.html
 */
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
auto create(OnSubscribe&& on_subscribe);

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
 * @tparam OnSubscribe is callback function to implement core logic of observable
 * @tparam Type is mostly deduced by OnSubscribe method if possible (if no any auto&& or templates in your callback)
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/create.html
 */
template<typename OnSubscribe, constraint::decayed_type Type = rpp::utils::extract_observer_type_t<rpp::utils::decayed_function_argument_t<OnSubscribe>>>
auto create(OnSubscribe&& on_subscribe)
{
    return create<Type>(std::forward<OnSubscribe>(on_subscribe));
}
} // namespace rpp::source