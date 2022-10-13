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

#include <rpp/operators/lift.hpp>                          // required due to operator uses lift
#include <rpp/operators/merge.hpp>
#include <rpp/operators/fwd/switch_on_next.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/utils/spinlock.hpp>


#include <atomic>
#include <memory>

IMPLEMENTATION_FILE(switch_on_next_tag);

namespace rpp::details
{
struct switch_on_next_state : public merge_state
{
    using merge_state::merge_state;
    
    composite_subscription current_inner_observable = rpp::composite_subscription::empty();
};

struct switch_on_next_on_completed_inner
{
    void operator()(const constraint::subscriber auto&           sub,
                    const std::shared_ptr<switch_on_next_state>& state) const
    {
        // 1 because decrement happens in composite_subscription_callback
        if (state->count_of_on_completed_needed.load(std::memory_order::acquire) == 1)
            sub.on_completed();
    }
};

using switch_on_next_on_next_inner = merge_forwarding_on_next;
using switch_on_next_on_error      = merge_on_error;

struct switch_on_next_on_next
{
    template<constraint::observable TObs>
    void operator()(const TObs&                                  new_observable,
                    const constraint::subscriber auto&           sub,
                    const std::shared_ptr<switch_on_next_state>& state) const
    {
        using ValueType = utils::extract_observable_type_t<TObs>;

        state->current_inner_observable.unsubscribe();
        state->current_inner_observable = state->children_subscriptions.make_child();
        state->current_inner_observable.add([state = std::weak_ptr{state}]
        {
            if (const auto locked = state.lock())
                locked->count_of_on_completed_needed.fetch_sub(1, std::memory_order::relaxed);
        });

        state->count_of_on_completed_needed.fetch_add(1, std::memory_order::relaxed);

        new_observable.subscribe(create_subscriber_with_state<ValueType>(state->current_inner_observable,
                                                                         switch_on_next_on_next_inner{},
                                                                         switch_on_next_on_error{},
                                                                         switch_on_next_on_completed_inner{},
                                                                         sub,
                                                                         state));
    }
};

using switch_on_next_on_completed_outer = merge_on_completed;

struct switch_on_next_state_with_serialized_spinlock : switch_on_next_state
{
    using switch_on_next_state::switch_on_next_state;

    // we can use spinlock there because 99.9% of time only one ever thread would send values from on_next (only one active observable), but we have small probability to get error from main observable immediately
    utils::spinlock spinlock{};
};

template<constraint::decayed_type Type>
struct switch_on_next_impl
{
    using ValueType = utils::extract_observable_type_t<Type>;

    template<constraint::subscriber_of_type<ValueType> TSub>
    auto operator()(TSub&& in_subscriber) const
    {
        auto state = std::make_shared<switch_on_next_state_with_serialized_spinlock>(in_subscriber.get_subscription());

        // change subscriber to serialized to avoid manual using of mutex
        auto subscriber = make_serialized_subscriber(std::forward<TSub>(in_subscriber), std::shared_ptr<utils::spinlock>{state, &state->spinlock});

        state->count_of_on_completed_needed.fetch_add(1, std::memory_order::relaxed);

        auto subscription = state->children_subscriptions.make_child();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  switch_on_next_on_next{},
                                                  switch_on_next_on_error{},
                                                  switch_on_next_on_completed_outer{},
                                                  std::move(subscriber),
                                                  std::move(state));
    }
};
} // namespace rpp::details
