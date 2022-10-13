//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                    TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/defs.hpp>                                    // RPP_NO_UNIQUE_ADDRESS
#include <rpp/operators/lift.hpp>                          // required due to operator uses lift
#include <rpp/operators/merge.hpp>                         // merge_state
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/combine_latest.hpp>            // own forwarding
#include <rpp/subscribers/constraints.hpp>                 // constraint::subscriber_of_type
#include <rpp/utils/spinlock.hpp>                          // spinlock

#include <algorithm>

IMPLEMENTATION_FILE(combine_latest_tag);

namespace rpp::details
{
/**
 * \brief The state that caches the values from all the observables and dispatches
 * latest caches to the observer. Note the emission is only sent to the observer when all
 * the observables at least emits once.
 */
template<typename TCombiner, constraint::decayed_type... Types>
struct combine_latest_state : public merge_state
{
    explicit combine_latest_state(const TCombiner& combiner, const composite_subscription& subscription_of_subscriber)
        : merge_state(subscription_of_subscriber)
        , combiner(combiner) {}

    // don't use NO_UNIQUE_ADDRESS there due to issue in MSVC base class becomes invalid
    /*NO_UNIQUE_ADDRESS*/ TCombiner     combiner;
    std::mutex                          values_mutex{};
    std::tuple<std::optional<Types>...> values{};
};

template<size_t I>
struct combine_latest_on_next
{
    template<typename TCombiner, constraint::decayed_type... Types>
    void operator()(auto&&                                                            value,
                    const auto&                                                       subscriber,
                    const std::shared_ptr<combine_latest_state<TCombiner, Types...>>& state) const
    {
        // mutex need to be locked during changing of values, generating new values and sending of new values due to we can't update value while we are sending old one
        std::scoped_lock lock{state->values_mutex};
        std::get<I>(state->values) = std::forward<decltype(value)>(value);

        std::apply([&](const auto&...cached_values)
                   {
                       if ((cached_values.has_value() && ...))
                           subscriber.on_next(state->combiner(cached_values.value()...));
                   },
                   state->values);
    }
};

using combine_latest_on_error     = merge_on_error;
using combine_latest_on_completed = merge_on_completed;

template<typename TCombiner, constraint::decayed_type... Types>
struct combine_latest_state_with_serialized_spinlock : combine_latest_state<TCombiner,Types...>
{
    using combine_latest_state<TCombiner,Types...>::combine_latest_state;

    // we can use spinlock there because 99.9% of time only one ever thread would send values from on_next serialized (due to values_mutex), but we have small probability to get error from another observable immediately
    utils::spinlock spinlock{};
};

/**
 * \brief "combine_latest" operator (an OperatorFn used by "lift").
 */
template<constraint::decayed_type Type, typename TCombiner, constraint::observable ...TOtherObservable>
struct combine_latest_impl
{
    RPP_NO_UNIQUE_ADDRESS TCombiner                       m_combiner;
    RPP_NO_UNIQUE_ADDRESS std::tuple<TOtherObservable...> m_other_observables;

private:
    static constexpr size_t s_index_of_source_type = 0;

    /**
     * \brief Templated helper function for subscribing to variadic 'other' observables.
     *
     * \param subscriber is the downstream subscriber.
     * \param state manages the cache of emission from the observables and coordinate dispatching.
     */
    template<size_t...I>
    void subscribe_other_observables(std::index_sequence<I...>,
                                     // Used in compile time for variadic expansion
                                     const auto& subscriber,
                                     const std::shared_ptr<combine_latest_state<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>>& state) const
    {
        // +1 because the first element in tuple is the current observable, and you want to subscribe to the 'other' observables.
        // (Use variadic expansion to iterate the observables)
        (subscribe_observable<I + 1>(std::get<I>(m_other_observables), subscriber, state), ...);
    }

    template<size_t I, constraint::observable TObservable>
    static void subscribe_observable(const TObservable& observable, const auto& subscriber, const std::shared_ptr<combine_latest_state<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>>& state)
    {
        using ValueType = utils::extract_observable_type_t<TObservable>;
        observable.subscribe(create_inner_subscriber<ValueType, I>(subscriber, state));
    }

    template<typename ValueType, size_t I>
    static auto create_inner_subscriber(auto&&                 subscriber,
                                        std::shared_ptr<combine_latest_state<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>> state)
    {
        auto subscription = state->children_subscriptions.make_child();
        return create_subscriber_with_state<ValueType>(std::move(subscription),
                                                       combine_latest_on_next<I>{},
                                                       combine_latest_on_error{},
                                                       combine_latest_on_completed{},
                                                       std::forward<decltype(subscriber)>(subscriber),
                                                       std::move(state));
    }


public:
    using DownstreamType = utils::decayed_invoke_result_t<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>;

    template<constraint::subscriber_of_type<DownstreamType> TSub>
    auto operator()(TSub&& in_subscriber) const
    {
        auto state = std::make_shared<combine_latest_state_with_serialized_spinlock<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>>(m_combiner, in_subscriber.get_subscription());
        // change subscriber to serialized to avoid manual using of mutex
        auto subscriber = make_serialized_subscriber(std::forward<TSub>(in_subscriber), std::shared_ptr<utils::spinlock>{state, &state->spinlock});

        state->count_of_on_completed_needed.store(sizeof...(TOtherObservable) + 1, std::memory_order::relaxed);

        // Subscribe to other observables and redirect on_next event to state
        subscribe_other_observables(std::index_sequence_for<TOtherObservable...>{}, subscriber, state);

        // Redirect values from this observable to the state for value composition
        return create_inner_subscriber<Type, s_index_of_source_type>(std::move(subscriber), std::move(state));
    }
};
} // namespace rpp::details
