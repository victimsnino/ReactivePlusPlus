//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observers/fwd.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/function_traits.hpp>

#include <concepts>
#include <type_traits>

namespace rpp::constraint
{
template<typename S, typename T>
concept observable_strategy = requires(const S& strategy, dynamic_observer<T>&& observer)
{
    {strategy.subscribe(std::move(observer))} -> std::same_as<void>;
};
}

namespace rpp::details::observables
{
template<constraint::decayed_type Type>
class dynamic_strategy;

template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class blocking_strategy;
}

namespace rpp
{
template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class observable;

/**
 * @brief Type-erased version of the `rpp::observable`. Any observable can be converted to dynamic_observable via `rpp::observable::as_dynamic` member function.
 * @details To provide type-erasure it uses `std::shared_ptr`. As a result it has worse performance.
 *
 * @tparam Type of value this obsevalbe can provide
 *
 * @ingroup observables
 */
template<constraint::decayed_type Type>
using dynamic_observable = observable<Type, details::observables::dynamic_strategy<Type>>;

/**
 * @brief `rpp::blocking_observable` blocks `subscribe` call till on_completed/on_error happens.
 */
template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
using blocking_observable = observable<Type, details::observables::blocking_strategy<Type, Strategy>>;
}

namespace rpp::utils
{
namespace details
{
    template<typename T>
    struct extract_observable_type
    {
        template<typename TT, typename Strategy>
        static TT deduce(const rpp::observable<TT, Strategy>&);

        using type = decltype(deduce(std::declval<std::decay_t<T>>()));
    };

    template<typename T>
    struct is_observable_t
    {
        template<typename TT, typename Strategy>
        static std::true_type is_observable(const rpp::observable<TT, Strategy>*);

        static std::false_type is_observable(...);

        static constexpr auto value = decltype(is_observable(std::declval<std::decay_t<T>*>()))::value;
    };

} // namespace details
template<typename T>
using extract_observable_type_t = typename details::extract_observable_type<std::decay_t<T>>::type;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T>
concept observable = rpp::utils::details::is_observable_t<std::decay_t<T>>::value;

template<typename Op, typename TObs>
concept operators = requires(const Op& op, TObs obs)
{
    {op(static_cast<TObs>(obs))} -> rpp::constraint::observable;
};

template<typename TObservable, typename... TObservables>
concept observables_of_same_type = rpp::constraint::observable<TObservable> &&
    (rpp::constraint::observable<TObservables> && ...) &&
    (std::same_as<rpp::utils::extract_observable_type_t<TObservable>, rpp::utils::extract_observable_type_t<TObservables>> && ...);
}