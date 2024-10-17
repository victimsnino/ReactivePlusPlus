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

#include <rpp/observables/details/disposable_strategy.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/utils.hpp>

namespace rpp::constraint
{
    template<typename S, typename T>
    concept observable_strategy = requires(const S& strategy, rpp::details::observers::fake_observer<T>&& observer) {
        {
            strategy.subscribe(std::move(observer))
        } -> std::same_as<void>;

        typename S::value_type;
        typename S::optimal_disposable_strategy;
        requires rpp::details::observables::constraint::disposable_strategy<typename S::optimal_disposable_strategy>;
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
        using value_type = Type;

        static void subscribe(const auto&) {}
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
    concept operator_lift_with_disposable_strategy = requires(const Op& op, rpp::details::observers::fake_observer<typename std::decay_t<Op>::template operator_traits<Type>::result_type>&& observer) {
        {
            op.template lift_with_disposable_strategy<Type, DisposableStrategy>(std::move(observer))
        } -> rpp::constraint::observer_of_type<Type>;
    };

    template<typename Op, typename Type, typename DisposableStrategy>
    concept operator_chain =
        requires() {
            typename std::decay_t<Op>::template operator_traits<Type>;
            typename std::decay_t<Op>::template operator_traits<Type>::result_type;
        }
        && details::observables::constraint::disposable_strategy<details::observables::deduce_updated_optimal_disposable_strategy_t<std::decay_t<Op>,
                                                                                                                                    typename details::observables::chain<details::observables::fake_strategy<Type>>::optimal_disposable_strategy>>
        && (operator_subscribe<std::decay_t<Op>, Type> || operator_lift<std::decay_t<Op>, Type> || operator_lift_with_disposable_strategy<std::decay_t<Op>, Type, DisposableStrategy>);

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
