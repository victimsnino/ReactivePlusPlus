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

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state


IMPLEMENTATION_FILE(combine_latest_tag);

namespace rpp::details
{

/**
 * \brief "combine_latest" operator (an OperatorFn used by "lift").
 */
template<constraint::decayed_type Type, typename TCombiner, constraint::observable ...TOtherObservable>
struct combine_latest_impl
{
    TCombiner m_combiner;
    std::tuple<TOtherObservable...> m_other_observables;

private:
    static constexpr size_t s_index_of_source_type = 0;

    using DownstreamType = utils::decayed_invoke_result_t<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>;

    /**
     * \brief The coordinator that caches the values from all the observables and dispatches latest caches to the
     * observer. Note the emission is only sent to the observer when all the observables at least emits once.
     */
    struct combine_latest_coordinator_t
    {
        explicit combine_latest_coordinator_t(const TCombiner& combiner) : m_combiner(combiner) {}

        // Not copyable nor movable.
        combine_latest_coordinator_t(const combine_latest_coordinator_t&) = delete;
        combine_latest_coordinator_t(combine_latest_coordinator_t&&) = delete;

        template<size_t I>
        void offer_next_value(auto&& value, const auto& subscriber)
        {
            std::scoped_lock<std::mutex> lock{m_mutex};
            std::get<I>(m_values) = std::forward<decltype(value)>(value);

            // If all the optional(s) have value, dispatch to downstream
            if (has_values_for_observer())
            {
                subscriber.on_next(combine_cached_values());
            }
        }

        void offer_completed(const auto& subscriber)
        {
            if (++m_completed_count == m_total_completed_count)
            {
                subscriber.on_completed();
            }
        }

    private:
        TCombiner m_combiner;

        std::mutex m_mutex;
        std::tuple<std::optional<Type>, std::optional<utils::extract_observable_type_t<TOtherObservable>>...> m_values{};
        std::atomic_size_t m_completed_count{0};

        const std::size_t m_total_completed_count = 1 + sizeof...(TOtherObservable);

        bool has_values_for_observer()
        {
            return std::apply([](const auto&... cached_values)
            {
                // Variadic expansion used
                return (cached_values.has_value() && ...);
            }, m_values);
        }

        auto combine_cached_values()
        {
            return std::apply([&](const auto&... cached_values)
            {
                return m_combiner(cached_values.value()...);
            }, m_values);
        }
    };

    /**
     * \brief Templated helper function for subscribing to variadic 'other' observables.
     *
     * \param observables is a variadic tuple. The first observable is the current observable and rest are the other observables.
     * \param subscriber is the downstream subscriber.
     * \param coordinator manages the cache of emission from the observables and coordinate dispatching.
     */
    template<size_t...I>
    void subscribe_other_observables(std::index_sequence<I...>, // Used in compile time for variadic expansion
                                     const auto& subscriber,
                                     const auto& coordinator) const
    {
        // +1 because the first element in tuple is the current observable, and you want to subscribe to the 'other' observables.
        // (Use variadic expansion to iterate the observables)
        (subscribe_observable<I + 1>(std::get<I>(m_other_observables), subscriber, coordinator), ...);
    }

    template<size_t I, constraint::observable TObservable>
    static void subscribe_observable(const TObservable& observable, const auto& subscriber, const auto& coordinator)
    {
        using ValueType = utils::extract_observable_type_t<TObservable>;
        observable.subscribe(create_inner_subscriber<ValueType, I>(subscriber, coordinator));
    }

    template<typename ValueType, size_t I>
    static auto create_inner_subscriber(auto&& subscriber, const auto& coordinator)
    {
        auto subscription = subscriber.get_subscription().make_child();
        return create_subscriber_with_state<ValueType>(
                    std::move(subscription),
                    [coordinator](auto&& value, const auto& subscriber)
                    {
                        coordinator->template offer_next_value<I>(std::forward<decltype(value)>(value), subscriber);
                    },
                    utils::forwarding_on_error{},
                    [coordinator](const auto& subscriber)
                    {
                        coordinator->offer_completed(subscriber);
                    },
                    std::forward<decltype(subscriber)>(subscriber));
    }

public:
    template<constraint::subscriber_of_type<DownstreamType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto coordinator = std::make_shared<combine_latest_coordinator_t>(m_combiner);

        // Subscribe to other observables and redirect on_next event to coordinator
        subscribe_other_observables(std::index_sequence_for<TOtherObservable...>{}, subscriber, coordinator);

        // Redirect values from this observable to the coordinator for value composition
        return create_inner_subscriber<Type, s_index_of_source_type>(subscriber, coordinator);
    }
};

} // namespace rpp::details
