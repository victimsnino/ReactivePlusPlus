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

#include <rpp/disposables/fwd.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/function_traits.hpp>
#include <rpp/utils/functors.hpp>

#include <concepts>
#include <exception>
#include <type_traits>

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
concept observer_strategy = requires(const S& const_strategy, S& strategy, const Type& v, const rpp::disposable_wrapper& disposable)
{
    const_strategy.on_next(v);
    const_strategy.on_next(Type{});
    const_strategy.on_error(std::exception_ptr{});
    const_strategy.on_completed();

    strategy.set_upstream(disposable);
    { strategy.is_disposed() } -> std::same_as<bool>;
};
} // namespace rpp::constraint

namespace rpp::details::observer
{
template<constraint::decayed_type Type>
class dynamic_strategy;

template<constraint::decayed_type                  Type,
         std::invocable<Type>                      OnNext,
         std::invocable<const std::exception_ptr&> OnError,
         std::invocable<>                          OnCompleted>
struct lambda_strategy;
} // namespace rpp::details

namespace rpp
{
template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
class base_observer;

/**
 * @brief Type-erased version of the rpp::base_observer. Any observer can be converted to dynamic_observer via rpp::base_observer::as_dynamic member function.
 * @details To provide type-erasure it uses std::shared_ptr. As a result it has worse performance, but it is ONLY way to copy observer.
 *
 * @tparam Type of value this observer can handle
 */
template<constraint::decayed_type Type>
using dynamic_observer = base_observer<Type, details::observer::dynamic_strategy<Type>>;

/**
 * @brief Observer specialized with passed callbacks. Most easiesest way to construct observer "on the fly" via lambdas and etc.
 *
 * @tparam Type of value this observer can handle
 * @tparam OnNext is type of callback to handle on_next(const Type&) and on_next(Type&&)
 * @tparam OnError is type of callback to handle on_error(const std::exception_ptr&)
 * @tparam OnCompleted is type of callback to handle on_completed()
 */
template<constraint::decayed_type Type, std::invocable<Type> OnNext,  std::invocable<const std::exception_ptr&> OnError, std::invocable<> OnCompleted>
using lambda_observer = base_observer<Type, details::observer::lambda_strategy<Type, OnNext, OnError, OnCompleted>>;

template<constraint::decayed_type Type,
         std::invocable<Type>                      OnNext,
         std::invocable<const std::exception_ptr&> OnError = rpp::utils::rethrow_error_t,
         std::invocable<>                          OnCompleted = rpp::utils::empty_function_t<>>
auto make_lambda_observer(OnNext&&      on_next,
                          OnError&&     on_error = {},
                          OnCompleted&& on_completed = {}) -> lambda_observer<Type,
                                                                         std::decay_t<OnNext>,
                                                                         std::decay_t<OnError>,
                                                                         std::decay_t<OnCompleted>>;

template<constraint::decayed_type Type,
         std::invocable<Type>                      OnNext,
         std::invocable<const std::exception_ptr&> OnError = rpp::utils::rethrow_error_t,
         std::invocable<>                          OnCompleted = rpp::utils::empty_function_t<>>
auto make_lambda_observer(const rpp::disposable_wrapper& d,
                          OnNext&&      on_next,
                          OnError&&     on_error = {},
                          OnCompleted&& on_completed = {}) -> lambda_observer<Type,
                                                                         std::decay_t<OnNext>,
                                                                         std::decay_t<OnError>,
                                                                         std::decay_t<OnCompleted>>;

template<typename                                  OnNext,
         std::invocable<const std::exception_ptr&> OnError = rpp::utils::rethrow_error_t,
         std::invocable<>                          OnCompleted = rpp::utils::empty_function_t<>,
         constraint::decayed_type                  Type = rpp::utils::decayed_function_argument_t<OnNext>>
    requires std::invocable<OnNext, Type>
auto make_lambda_observer(OnNext&&      on_next,
                          OnError&&     on_error = {},
                          OnCompleted&& on_completed = {})
{
    return make_lambda_observer<Type>(std::forward<OnNext>(on_next), std::forward<OnError>(on_error), std::forward<OnCompleted>(on_completed));
}

template<typename                                  OnNext,
         std::invocable<const std::exception_ptr&> OnError = rpp::utils::rethrow_error_t,
         std::invocable<>                          OnCompleted = rpp::utils::empty_function_t<>,
         constraint::decayed_type                  Type = rpp::utils::decayed_function_argument_t<OnNext>>
    requires std::invocable<OnNext, Type>
auto make_lambda_observer(const rpp::disposable_wrapper& d,
                          OnNext&&      on_next,
                          OnError&&     on_error = {},
                          OnCompleted&& on_completed = {})
{
    return make_lambda_observer<Type>(d, std::forward<OnNext>(on_next), std::forward<OnError>(on_error), std::forward<OnCompleted>(on_completed));
}
} // namespace rpp


namespace rpp::utils
{
namespace details
{
    template<typename T>
    struct extract_observer_type : std::false_type{};

    template<typename TT, typename Strategy>
    struct extract_observer_type<rpp::base_observer<TT, Strategy>> : std::true_type
    {
        using type = TT;
    };

} // namespace details
template<typename T>
using extract_observer_type_t = typename details::extract_observer_type<T>::type;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T>
concept observer = rpp::utils::details::extract_observer_type<std::decay_t<T>>::value;
}