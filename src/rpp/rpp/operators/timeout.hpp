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

#include <rpp/operators/lift.hpp>          // required due to operator uses lift
#include <rpp/operators/details/early_unsubscribe.hpp>
#include <rpp/operators/details/serialized_subscriber.hpp>
#include <rpp/operators/details/subscriber_with_state.hpp>
#include <rpp/operators/fwd/timeout.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/exceptions.hpp>

#include <rpp/utils/spinlock.hpp>

#include <atomic>

IMPLEMENTATION_FILE(timeout_tag);

namespace rpp::details
{
struct timeout_state : early_unsubscribe_state
{
    using early_unsubscribe_state::early_unsubscribe_state;

    std::atomic<schedulers::time_point> last_emission_time{};

    static constexpr schedulers::time_point s_timeout_reached = schedulers::time_point::min();
};

template<typename Worker>
struct timeout_on_next
{
    template<typename Value>
    void operator()(Value&& v, const auto& subscriber, const std::shared_ptr<timeout_state>& state) const
    {
        // actually there is only 2 threads: this one and thread in "timeout schedulable".
        auto last_emission_time = state->last_emission_time.load(std::memory_order_relaxed);
        while (last_emission_time != timeout_state::s_timeout_reached)
        {
            // only one possible way why compare_exchange_strong fails -> timeout schedulable does its action
            if (state->last_emission_time.compare_exchange_strong(last_emission_time, Worker::now(), std::memory_order_acq_rel))
            {
                subscriber.on_next(std::forward<Value>(v));
                return;
            }
        }
    }
};

using timeout_on_error     = early_unsubscribe_on_error;
using timeout_on_completed = early_unsubscribe_on_completed;

struct timeout_state_with_serialized_spinlock : timeout_state
{
    using timeout_state::timeout_state;

    // spinlock because most part of time there is only one thread would be active
    utils::spinlock spinlock{};
};

template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct timeout_impl
{
    schedulers::duration period;
    TScheduler           scheduler;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& in_subscriber) const
    {
        auto state = std::make_shared<timeout_state_with_serialized_spinlock>(in_subscriber.get_subscription());
        // change subscriber to serialized to avoid manual using of mutex
        auto subscriber = make_serialized_subscriber(std::forward<TSub>(in_subscriber),
                                                     std::shared_ptr<utils::spinlock>{state, &state->spinlock});

        const auto worker = scheduler.create_worker(state->children_subscriptions);
        state->last_emission_time.store(worker.now(), std::memory_order_relaxed);

        const auto last_emission_time = state->last_emission_time.load(std::memory_order_relaxed);
        worker.schedule(last_emission_time + period,
                        [period = period, prev_emission_time = last_emission_time, subscriber, state]() mutable -> schedulers::optional_duration
                        {
                            // last emission time still same value -> timeout reached, else -> prev_emission_time would be update to actual emission time
                            if (state->last_emission_time.compare_exchange_strong(prev_emission_time, timeout_state::s_timeout_reached, std::memory_order_acq_rel))
                                return time_is_out(state, subscriber);

                            // if we still need to wait a bit more -> let's wait
                            if (const auto diff_to_schedule = (prev_emission_time + period) - decltype(worker)::now(); diff_to_schedule > rpp::schedulers::duration{0})
                                return diff_to_schedule;

                            // looks like even "new" end of time is before current time
                            return time_is_out(state, subscriber);
                        });

        return create_subscriber_with_state<Type>(state->children_subscriptions,
                                                  timeout_on_next<decltype(worker)>{},
                                                  timeout_on_error{},
                                                  timeout_on_completed{},
                                                  std::move(subscriber),
                                                  std::move(state));
    }

private:
    static auto time_is_out(const auto& state, const auto& subscriber)
    {
        state->children_subscriptions.unsubscribe();
        subscriber.on_error(std::make_exception_ptr(utils::timeout{"Timeout reached"}));
        return schedulers::optional_duration{};
    }
};
} // namespace rpp::details
