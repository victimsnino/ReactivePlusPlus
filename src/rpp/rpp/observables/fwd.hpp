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

namespace rpp::constraint
{
template<typename S, typename T>
concept observable_strategy = requires(const S& strategy, details::fake_observer<T>&& observer)
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

namespace rpp::utils::details
{
template<typename TObservable>
struct is_observable_t
{
    template<typename T, typename Strategy>
    constexpr static std::true_type  deduce(const rpp::observable<T, Strategy>*);
    constexpr static std::false_type deduce(...);

    using type = decltype(deduce(std::declval<std::decay_t<TObservable>*>()));
};

} // namespace rpp::utils::details

namespace rpp::constraint
{
template<typename T>
concept observable = rpp::utils::details::is_observable_t<std::decay_t<T>>::type::value;
}

namespace rpp
{
template<rpp::constraint::observable OriginalObservable, rpp::constraint::subject Subject>
class connectable_observable;
}

namespace rpp::utils
{
namespace details
{
    template<rpp::constraint::observable TObservable>
    struct extract_observable_type
    {
        template<typename T, typename Strategy>
        constexpr static T deduce(const rpp::observable<T, Strategy>&);

        using type = decltype(deduce(std::declval<std::decay_t<TObservable>>()));
    };

} // namespace details

template<typename T>
using extract_observable_type_t = typename details::extract_observable_type<std::decay_t<T>>::type;
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
concept operator_base = requires(const Op& op) {
    typename std::decay_t<Op>::template result_value<Type>;
    requires details::observables::constraint::disposable_strategy<details::observables::deduce_updated_disposable_strategy<std::decay_t<Op>, typename observable_chain_strategy<details::observables::dynamic_strategy<Type>>::expected_disposable_strategy>>;
};

template<typename Op, typename Type>
concept operator_subscribe = operator_base<Op, Type> && requires(const Op& op, dynamic_observer<typename std::decay_t<Op>::template result_value<Type>>&& observer, const observable_chain_strategy<details::observables::dynamic_strategy<Type>>& chain)
{
    {op.subscribe(std::move(observer), chain)};
};

template<typename Op, typename Type>
concept operator_lift = operator_base<Op, Type> && requires(const Op& op, dynamic_observer<typename std::decay_t<Op>::template result_value<Type>>&& observer)
{
    {op.template lift<Type>(std::move(observer))} -> rpp::constraint::observer_of_type<Type>;
};

template<typename Op, typename Type, typename DisposableStrategy>
concept operator_lift_with_disposable_strategy = operator_base<Op, Type> && requires(const Op& op, dynamic_observer<typename std::decay_t<Op>::template result_value<Type>>&& observer)
{
    {op.template lift_with_disposable_strategy<Type, DisposableStrategy>(std::move(observer))} -> rpp::constraint::observer_of_type<Type>;
};

template<typename Op, typename Type, typename DisposableStrategy>
concept operator_chain = operator_subscribe<Op, Type> || operator_lift<Op, Type> || operator_lift_with_disposable_strategy<Op, Type, DisposableStrategy>;

template<typename TObservable, typename... TObservables>
concept observables_of_same_type = rpp::constraint::observable<TObservable> &&
    (rpp::constraint::observable<TObservables> && ...) &&
    (std::same_as<rpp::utils::extract_observable_type_t<TObservable>, rpp::utils::extract_observable_type_t<TObservables>> && ...);
}
