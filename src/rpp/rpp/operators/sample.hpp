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

#include <rpp/operators/details/early_unsubscribe.hpp>
#include <rpp/operators/details/serialized_subscriber.hpp>
#include <rpp/operators/fwd/sample.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/spinlock.hpp>

#include <mutex>
#include <optional>

IMPLEMENTATION_FILE(sample_tag);

namespace rpp::details
{
template<constraint::decayed_type Type>
struct sample_state : early_unsubscribe_state
{
    using early_unsubscribe_state::early_unsubscribe_state;

    std::mutex          value_mutex{};
    std::optional<Type> value{};
};

template<constraint::decayed_type Type>
struct sample_state_with_serialized_spinlock : sample_state<Type>
{
    using sample_state<Type>::sample_state;

    utils::spinlock spinlock{};
};

struct sample_on_next
{
    template<typename Value>
    void operator()(Value&& value, const auto&, const std::shared_ptr<sample_state<std::decay_t<Value>>>& state) const
    {
        std::lock_guard lock{state->value_mutex};
        state->value.emplace(std::forward<Value>(value));
    }
};

using sample_on_error = early_unsubscribe_on_error;

struct sample_on_completed
{
    void operator()(const auto& subscriber, const auto& state) const
    {
        state->children_subscriptions.unsubscribe();

        {
            std::lock_guard lock{state->value_mutex};
            if (state->value.has_value())
                subscriber.on_next(std::move(state->value.value()));
        }
        subscriber.on_completed();
    }
};

template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct sample_impl
{
    schedulers::duration period;
    TScheduler           scheduler;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& in_subscriber) const
    {
        auto state = std::make_shared<sample_state_with_serialized_spinlock<Type>>(in_subscriber.get_subscription());
        // change subscriber to serialized to avoid manual using of mutex
        auto subscriber = make_serialized_subscriber(std::forward<TSub>(in_subscriber),
                                                     std::shared_ptr<utils::spinlock>{state, &state->spinlock});

        scheduler.create_worker(state->children_subscriptions)
                 .schedule(period,
                           [period = period, subscriber = subscriber, state]() -> rpp::schedulers::optional_duration
                           {
                               std::optional<Type> extracted{};
                               {
                                   std::lock_guard lock{state->value_mutex};
                                   std::swap(extracted, state->value);
                               }
                               if (extracted.has_value())
                                   subscriber.on_next(std::move(extracted.value()));
                               return period;
                           });

        return create_subscriber_with_state<Type>(state->children_subscriptions,
                                                  sample_on_next{},
                                                  sample_on_error{},
                                                  sample_on_completed{},
                                                  std::move(subscriber),
                                                  std::move(state));
    }
};
} // namespace rpp::details
