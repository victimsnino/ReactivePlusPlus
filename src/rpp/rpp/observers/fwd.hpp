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

#include <exception>

namespace rpp::constraint
{
/**
 * @brief Concept to define strategy to override observer behavior. Strategy must be able to handle all observer's
 * callbacks: on_next/on_error/on_completed
 *
 * @tparam S is Strategy
 * @tparam Type is type of value observer would obtain
 */
template<typename S, typename Type>
concept observer_strategy = requires(const S& strategy, const Type& v)
{
    strategy.on_next(v);
    strategy.on_next(Type{});
    strategy.on_error(std::exception_ptr{});
    strategy.on_completed();
};
} // namespace rpp::constraint

namespace rpp::details
{
template<constraint::decayed_type Type>
class dynamic_strategy;

template<constraint::decayed_type Type, std::invocable<Type> OnNext,  std::invocable<const std::exception_ptr&> OnError, std::invocable<> OnCompleted>
struct lambda_strategy;
} // namespace rpp::details

namespace rpp
{
/**
 * @brief Base class for any observer used in RPP. It handles core callbacks of observers. Objects of this class would
 * be passed to subscribe of observable
 * 
 * @warning By default base_observer is not copyable, only movable. If you need to COPY your observer, you need to convert it to rpp::dynamic_observer via rpp::base_observer::as_dynamic
 *
 * @tparam Type of value this observer can handle
 * @tparam Strategy used to provide logic over observer's callbacks
 */
template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
class base_observer;

/**
 * @brief Type-erased version of the rpp::base_observer. Any observer can be converted to dynamic_observer via rpp::base_observer::as_dynamic member function.
 * @details To provide type-erasure it uses std::shared_ptr. As a result it has worser performance, but it is ONLY way to copy observer.
 * 
 * @tparam Type of value this observer can handle
 */
template<constraint::decayed_type Type>
using dynamic_observer = base_observer<Type, details::dynamic_strategy<Type>>;

/**
 * @brief Observer specialized with passed callbacks. Most easiesest way to construct observer "on the fly" via lambdas and etc.
 * 
 * @tparam Type of value this observer can handle 
 * @tparam OnNext is type of callback to handle on_next(const Type&) and on_next(Type&&)
 * @tparam OnError is type of callback to handle on_error(const std::exception_ptr&)
 * @tparam OnCompleted is type of callback to handle on_completed()
 */
template<constraint::decayed_type Type, std::invocable<Type> OnNext,  std::invocable<const std::exception_ptr&> OnError, std::invocable<> OnCompleted>
using lambda_observer = base_observer<Type, details::lambda_strategy<Type, OnNext, OnError, OnCompleted>>;

template<constraint::decayed_type Type,
         std::invocable<Type> OnNext,
         std::invocable<const std::exception_ptr&> OnError,
         std::invocable<> OnCompleted>
auto make_lambda_observer(OnNext&&      on_next,
                          OnError&&     on_error,
                          OnCompleted&& on_completed) -> lambda_observer<Type,
                                                                         std::decay_t<OnNext>,
                                                                         std::decay_t<OnError>,
                                                                         std::decay_t<OnCompleted>>;
} // namespace rpp
