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

#include <algorithm>

#include <rpp/operators/fwd/combine_latest.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/defs.hpp>


#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state


IMPLEMENTATION_FILE(combine_latest_tag);

namespace rpp::details
{
/**
 * \brief The state that caches the values from all the observables and dispatches
 * latest caches to the observer. Note the emission is only sent to the observer when all
 * the observables at least emits once.
 */
template<typename TCombiner, constraint::decayed_type... Types>
struct combine_latest_state
{
    explicit combine_latest_state(const TCombiner& combiner)
        : combiner(combiner) {}

    RPP_NO_UNIQUE_ADDRESS TCombiner     combiner;
    std::mutex                          mutex;
    std::tuple<std::optional<Types>...> values{};
    std::atomic_size_t                  completed_count{0};
    static constexpr std::size_t        s_total_completed_count = sizeof...(Types);
};

template<size_t I>
struct combine_latest_on_next
{
    template<typename TCombiner, constraint::decayed_type... Types>
    void operator()(auto&&                                                            value,
                    const auto&                                                       subscriber,
                    const std::shared_ptr<combine_latest_state<TCombiner, Types...>>& state) const
    {
        // mutex need to be locked during changing of values, generating new values and
        // sending of values to satisfy observable contract
        std::scoped_lock lock{state->mutex};
        std::get<I>(state->values) = std::forward<decltype(value)>(value);

        std::apply([&](const auto&...cached_values)
                   {
                       if ((cached_values.has_value() && ...))
                           subscriber.on_next(state->combiner(cached_values.value()...));
                   },
                   state->values);
    }
};

struct combine_latest_on_completed
{
    template<typename TCombiner, constraint::decayed_type... Types>

    void operator()(const auto&                                                       subscriber,
                    const std::shared_ptr<combine_latest_state<TCombiner, Types...>>& state) const
    {
        const auto current_completed = state->completed_count.fetch_add(1, std::memory_order_acq_rel) + 1;
        if (current_completed == state->s_total_completed_count)
            subscriber.on_completed();
    }
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
    using State = combine_latest_state<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>;

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
                                     const auto& state) const
    {
        // +1 because the first element in tuple is the current observable, and you want to subscribe to the 'other' observables.
        // (Use variadic expansion to iterate the observables)
        (subscribe_observable<I + 1>(std::get<I>(m_other_observables), subscriber, state), ...);
    }

    template<size_t I, constraint::observable TObservable>
    static void subscribe_observable(const TObservable& observable, const auto& subscriber, const auto& state)
    {
        using ValueType = utils::extract_observable_type_t<TObservable>;
        observable.subscribe(create_inner_subscriber<ValueType, I>(subscriber, state));
    }

    template<typename ValueType, size_t I>
    static auto create_inner_subscriber(auto&&                 subscriber,
                                        std::shared_ptr<State> state)
    {
        auto subscription = subscriber.get_subscription().make_child();
        return create_subscriber_with_state<ValueType>(std::move(subscription),
                                                       combine_latest_on_next<I>{},
                                                       utils::forwarding_on_error{},
                                                       combine_latest_on_completed{},
                                                       std::forward<decltype(subscriber)>(subscriber),
                                                       std::move(state));
    }


public:
    using DownstreamType = utils::decayed_invoke_result_t<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>;

    template<constraint::subscriber_of_type<DownstreamType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<State>(m_combiner);

        // Subscribe to other observables and redirect on_next event to state
        subscribe_other_observables(std::index_sequence_for<TOtherObservable...>{}, subscriber, state);

        // Redirect values from this observable to the state for value composition
        return create_inner_subscriber<Type, s_index_of_source_type>(std::forward<TSub>(subscriber), std::move(state));
    }
};
} // namespace rpp::details
