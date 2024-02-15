//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/details/disposable_strategy.hpp>
#include <rpp/observers/fwd.hpp>
#include <rpp/subjects/fwd.hpp>

#include <rpp/utils/constraints.hpp>
#include <rpp/utils/utils.hpp>

namespace rpp::constraint
{
template<typename S, typename T>
concept observable_strategy = requires(const S& strategy, rpp::details::observers::fake_observer<T>&& observer)
{
    {strategy.subscribe(std::move(observer))} -> std::same_as<void>;
    typename S::value_type;
};
}

namespace rpp::details::observables
{
template<rpp::constraint::decayed_type Type>
class dynamic_strategy;

template<rpp::constraint::decayed_type Type, rpp::constraint::observable_strategy<Type> Strategy>
class blocking_strategy;

template<rpp::constraint::decayed_type Type>
struct fake_strategy
{
    using value_type = Type;

    static void subscribe(const auto&) {}
};
}

namespace rpp
{
template<typename TStrategy, typename... TStrategies>
class observable_chain_strategy;

template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class observable;

template<constraint::decayed_type Type>
class dynamic_observable;

template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class blocking_observable;

template<constraint::decayed_type KeyType, constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class grouped_observable;
}

namespace rpp::constraint
{
template<typename T>
concept observable = rpp::utils::is_base_of_v<T, rpp::observable>;
}

namespace rpp
{
template<rpp::constraint::observable OriginalObservable, rpp::constraint::subject Subject>
class connectable_observable;
}

namespace rpp::utils
{
template<typename T>
using extract_observable_type_t = typename rpp::utils::extract_base_type_params_t<T, rpp::observable>::template type_at_index_t<0>;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T, typename Type>
concept observable_of_type = observable<T> && std::same_as<utils::extract_observable_type_t<T>, std::decay_t<Type>>;

template<typename Op, typename TObs>
concept operator_observable_transform = requires(const Op& op, TObs obs)
{
    {op(static_cast<TObs>(obs))} -> rpp::constraint::observable;
};

template<typename Op, typename Type>
concept operator_base = requires(const Op& op) { typename std::decay_t<Op>::template operator_traits<Type>; } && details::observables::constraint::disposable_strategy<details::observables::deduce_updated_disposable_strategy<std::decay_t<Op>, typename observable_chain_strategy<details::observables::fake_strategy<Type>>::expected_disposable_strategy>>;

template<typename Op, typename Type>
concept operator_subscribe = operator_base<Op, Type> && requires(const Op& op, rpp::details::observers::fake_observer<typename std::decay_t<Op>::template operator_traits<Type>::result_type>&& observer, const observable_chain_strategy<details::observables::fake_strategy<Type>>& chain)
{
    {op.subscribe(std::move(observer), chain)};
};

template<typename Op, typename Type>
concept operator_lift = operator_base<Op, Type> && requires(const Op& op, rpp::details::observers::fake_observer<typename std::decay_t<Op>::template operator_traits<Type>::result_type>&& observer)
{
    {op.template lift<Type>(std::move(observer))} -> rpp::constraint::observer_of_type<Type>;
};

template<typename Op, typename Type, typename DisposableStrategy>
concept operator_lift_with_disposable_strategy = operator_base<Op, Type> && requires(const Op& op, rpp::details::observers::fake_observer<typename std::decay_t<Op>::template operator_traits<Type>::result_type>&& observer)
{
    {op.template lift_with_disposable_strategy<Type, DisposableStrategy>(std::move(observer))} -> rpp::constraint::observer_of_type<Type>;
};

template<typename Op, typename Type, typename DisposableStrategy>
concept operator_chain = operator_base<std::decay_t<Op>, Type>
    && requires { typename std::decay_t<Op>::template operator_traits<Type>::result_type; }
    && (operator_subscribe<std::decay_t<Op>, Type> || operator_lift<std::decay_t<Op>, Type> || operator_lift_with_disposable_strategy<std::decay_t<Op>, Type, DisposableStrategy>);

template<typename TObservable, typename... TObservables>
concept observables_of_same_type = rpp::constraint::observable<TObservable> &&
    (rpp::constraint::observable<TObservables> && ...) &&
    (std::same_as<rpp::utils::extract_observable_type_t<TObservable>, rpp::utils::extract_observable_type_t<TObservables>> && ...);
}

#define RPP_CHECK_IF_TRAIT_ASSERTS_SATISFIED(Op, Type)                                              \
    /* operator_traits can be instantiated if all inner static_asserts are fine*/ \
    if constexpr (requires { { typename std::decay_t<Op>::template operator_traits<Type>{}}; })
