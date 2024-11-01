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

#include <rpp/observables/details/disposables_strategy.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/utils.hpp>

namespace rpp::constraint
{
    /**
     * @concept observable_strategy
     * @brief A concept that defines the requirements for an observable strategy.
     *
     * This concept ensures that a type `S` meets the following criteria:
     * - It has a `subscribe` method that accepts observer of type `T` and returns `void`.
     * - It defines a nested type `value_type` to represent the type of values emitted by the observable.
     * - It defines a nested type `optimal_disposables_strategy` to define the optimal disposables strategy observer could/should use to handle current observable properly.
     *
     * @tparam S The type to be checked against the concept.
     * @tparam T The type of the values emitted by the observable.
     *
     * @ingroup observables
     */
    template<typename S, typename T>
    concept observable_strategy = requires(const S& strategy, rpp::details::observers::fake_observer<T>&& observer) {
        {
            strategy.subscribe(std::move(observer))
        } -> std::same_as<void>;

        typename S::value_type;
        typename S::optimal_disposables_strategy;
        requires rpp::details::observables::constraint::disposables_strategy<typename S::optimal_disposables_strategy>;
    };
} // namespace rpp::constraint

namespace rpp::details::observables
{
    template<rpp::constraint::decayed_type Type>
    class dynamic_strategy;

    template<rpp::constraint::decayed_type Type, rpp::constraint::observable_strategy<Type> Strategy>
    class blocking_strategy;

    template<rpp::constraint::decayed_type Type>
    struct fake_strategy
    {
        using value_type                   = Type;
        using optimal_disposables_strategy = rpp::details::observables::fixed_disposables_strategy<0>;

        consteval static void subscribe(const auto&) {}
    };
} // namespace rpp::details::observables

namespace rpp::details::observables
{
    template<typename TStrategy, typename... TStrategies>
    class chain;
} // namespace rpp::details::observables

namespace rpp
{
    template<rpp::constraint::decayed_type Type, rpp::constraint::observable_strategy<Type> Strategy>
    class observable;
} // namespace rpp


namespace rpp::constraint
{
    template<typename T>
    concept observable = rpp::utils::is_base_of_v<T, rpp::observable>;
} // namespace rpp::constraint

namespace rpp
{
    template<rpp::constraint::observable OriginalObservable, typename Subject>
    class connectable_observable;
} // namespace rpp

namespace rpp::utils
{
    template<typename T>
    using extract_observable_type_t = typename rpp::utils::extract_base_type_params_t<T, rpp::observable>::template type_at_index_t<0>;
} // namespace rpp::utils

namespace rpp::constraint
{
    template<typename T, typename Type>
    concept observable_of_type = observable<T> && std::same_as<utils::extract_observable_type_t<T>, std::decay_t<Type>>;

    template<typename TObservable, typename... TObservables>
    concept observables_of_same_type = rpp::constraint::observable<TObservable> && (rpp::constraint::observable<TObservables> && ...) && (std::same_as<rpp::utils::extract_observable_type_t<TObservable>, rpp::utils::extract_observable_type_t<TObservables>> && ...);

    template<typename Op, typename Type>
    concept operator_subscribe = requires(const Op& op, rpp::details::observers::fake_observer<typename std::decay_t<Op>::template operator_traits<Type>::result_type>&& observer, const details::observables::chain<details::observables::fake_strategy<Type>>& chain) {
        {
            op.subscribe(std::move(observer), chain)
        };
    };

    template<typename Op, typename Type>
    concept operator_lift = requires(const Op& op, rpp::details::observers::fake_observer<typename std::decay_t<Op>::template operator_traits<Type>::result_type>&& observer) {
        {
            op.template lift<Type>(std::move(observer))
        } -> rpp::constraint::observer_of_type<Type>;
    };

    template<typename Op, typename Type, typename DisposableStrategy>
    concept operator_lift_with_disposables_strategy = requires(const Op& op, rpp::details::observers::fake_observer<typename std::decay_t<Op>::template operator_traits<Type>::result_type>&& observer) {
        {
            op.template lift_with_disposables_strategy<Type, DisposableStrategy>(std::move(observer))
        } -> rpp::constraint::observer_of_type<Type>;
    };

    template<typename Op, typename Type, typename DisposableStrategy>
    concept operator_chain =
        requires() {
            typename std::decay_t<Op>::template operator_traits<Type>;
            typename std::decay_t<Op>::template operator_traits<Type>::result_type;
            typename std::decay_t<Op>::template updated_optimal_disposables_strategy<typename details::observables::chain<details::observables::fake_strategy<Type>>::optimal_disposables_strategy>;
        }
        && details::observables::constraint::disposables_strategy<typename std::decay_t<Op>::template updated_optimal_disposables_strategy<typename details::observables::chain<details::observables::fake_strategy<Type>>::optimal_disposables_strategy>>
        && (operator_subscribe<std::decay_t<Op>, Type> || operator_lift<std::decay_t<Op>, Type> || operator_lift_with_disposables_strategy<std::decay_t<Op>, Type, DisposableStrategy>);

} // namespace rpp::constraint

namespace rpp
{
    template<constraint::decayed_type Type>
    class dynamic_observable;

    template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
    class blocking_observable;

    template<constraint::decayed_type KeyType, constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
    class grouped_observable;

    template<constraint::decayed_type Type, rpp::constraint::observable_of_type<Type>... Observables>
    class variant_observable;
} // namespace rpp
