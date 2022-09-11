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

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/merge.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/observers/state_observer.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/operators/details/early_unsubscribe.hpp>
#include <rpp/operators/details/serialized_subscriber.hpp>

#include <array>
#include <atomic>
#include <memory>

IMPLEMENTATION_FILE(merge_tag);

namespace rpp::details
{
struct merge_state : early_unsubscribe_state
{
    using early_unsubscribe_state::early_unsubscribe_state;

    std::atomic_size_t count_of_on_completed_needed{};
};

using merge_forwarding_on_next = utils::forwarding_on_next;
using merge_on_error           = early_unsubscribe_on_error;

struct merge_on_completed
{
    void operator()(const constraint::subscriber auto&  sub,
                    const std::shared_ptr<merge_state>& state) const
    {
        if (state->count_of_on_completed_needed.fetch_sub(1, std::memory_order::acq_rel) == 1)
            sub.on_completed();
    }
};

struct merge_on_next
{
    template<constraint::observable TObs>
    void operator()(const TObs&                         new_observable,
                    const constraint::subscriber auto&  sub,
                    const std::shared_ptr<merge_state>& state) const
    {
        using ValueType = utils::extract_observable_type_t<TObs>;

        state->count_of_on_completed_needed.fetch_add(1, std::memory_order::relaxed);

        new_observable.subscribe(create_subscriber_with_state<ValueType>(state->children_subscriptions.make_child(),
                                                                         merge_forwarding_on_next{},
                                                                         merge_on_error{},
                                                                         merge_on_completed{},
                                                                         sub,
                                                                         state));
    }
};

struct merge_state_with_mutex : merge_state
{
    using merge_state::merge_state;

    std::mutex mutex{};
};

template<constraint::decayed_type Type>
struct merge_impl
{
    using ValueType = utils::extract_observable_type_t<Type>;

    template<constraint::subscriber_of_type<ValueType> TSub>
    auto operator()(TSub&& in_subscriber) const
    {
        auto state = std::make_shared<merge_state_with_mutex>(in_subscriber.get_subscription());
        // change subscriber to serialized to avoid manual using of mutex
        auto subscriber = make_serialized_subscriber(std::forward<TSub>(in_subscriber), std::shared_ptr<std::mutex>{state, &state->mutex});

        state->count_of_on_completed_needed.fetch_add(1, std::memory_order::relaxed);

        auto subscription = state->children_subscriptions.make_child();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  merge_on_next{},
                                                  merge_on_error{},
                                                  merge_on_completed{},
                                                  std::move(subscriber),
                                                  std::move(state));
    }
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> ... TObservables>
auto merge_with_impl(TObservables&&... observables)
{
    return source::just(std::forward<TObservables>(observables).as_dynamic()...).merge();
}
} // namespace rpp::details
