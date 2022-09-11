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

#include <rpp/defs.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/operators/fwd/take_until.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/utils/spinlock.hpp>

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state

#include <mutex>

IMPLEMENTATION_FILE(take_until_tag);

namespace rpp::details
{
using take_until_state = early_unsubscribe_state;

using take_until_on_next      = merge_forwarding_on_next;
using take_until_on_error     = merge_on_error;
using take_until_on_completed = early_unsubscribe_on_completed;


/**
 * Functor (type-erasure) of throttler (trigger observable) for on_next operator.
 */
struct take_until_throttler_on_next
{
    void operator()(auto&&, const auto& subscriber, const std::shared_ptr<take_until_state>& state) const
    {
        // Unsubscribe all sources due to we obtained "stop event"
        state->children_subscriptions.unsubscribe();
        subscriber.on_completed();
    }
};

using take_until_throttler_on_error     = take_until_on_error;
using take_until_throttler_on_completed = take_until_on_completed;

struct take_until_state_with_serialized_spinlock : take_until_state
{
    using take_until_state::take_until_state;

    // we can use spinlock there because 99.9% of time only one ever thread would send values from on_next (main observable), but we have small probability to get error from "until observable" immediately
    utils::spinlock spinlock{};
};
/**
 * \brief "combine_latest" operator (an OperatorFn used by "lift").
 */
template<constraint::decayed_type Type, constraint::observable TTriggerObservable>
struct take_until_impl
{
    using TriggerType = utils::extract_observable_type_t<TTriggerObservable>;

    TTriggerObservable m_until_observable;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& in_subscriber) const
    {
        auto state = std::make_shared<take_until_state_with_serialized_spinlock>(in_subscriber.get_subscription());
        // change subscriber to serialized to avoid manual using of mutex
        auto subscriber = make_serialized_subscriber(std::forward<TSub>(in_subscriber), std::shared_ptr<utils::spinlock>{state, &state->spinlock});

        // Subscribe to trigger observable
        m_until_observable.subscribe(create_subscriber_with_state<TriggerType>(state->children_subscriptions.make_child(),
                                                                               take_until_throttler_on_next{},
                                                                               take_until_throttler_on_error{},
                                                                               take_until_throttler_on_completed{},
                                                                               subscriber,
                                                                               state));

        auto subscription = state->children_subscriptions.make_child();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  take_until_on_next{},
                                                  take_until_on_error{},
                                                  take_until_on_completed{},
                                                  std::move(subscriber),
                                                  std::move(state));
    }
};

} // namespace rpp::details
