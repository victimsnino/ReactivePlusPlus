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
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/delay.hpp>                     // own forwarding
#include <rpp/subscribers/constraints.hpp>                 // constraint::subscriber_of_type

IMPLEMENTATION_FILE(delay_tag);

namespace rpp::details
{
/**
 * Functor (type-erasure) of "delay" for on_next operator.
 */
struct delay_on_next
{
    schedulers::duration delay;

    void operator()(auto&& value, const auto& subscriber, const auto& worker) const
    {
        worker.schedule(delay,
                        [value = std::forward<decltype(value)>(value), subscriber]()
                        {
                            subscriber.on_next(std::move(value));
                            return schedulers::optional_duration{};
                        });
    }
};

/**
 * Functor (type-erasure) of "delay" for on_error operator.
 */
struct delay_on_error
{
    void operator()(const std::exception_ptr& err, const auto& subscriber, const auto& worker) const
    {
        // on-error must be delivered as soon as possible
        worker.schedule([err, subscriber]()
                        {
                            subscriber.on_error(err);
                            return schedulers::optional_duration{};
                        });
    }
};

/**
 * Functor (type-erasure) of "delay" for on_completed operator.
 */
struct delay_on_completed
{
    schedulers::duration delay;

    void operator()(const auto& subscriber, const auto& worker) const
    {
        worker.schedule(delay,
                        [subscriber]()
                        {
                            subscriber.on_completed();
                            return schedulers::optional_duration{};
                        });
    }
};

/**
 * \brief Functor of OperatorFn for "combine_latest" operator (used by "lift").
 */
template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct delay_impl
{
    RPP_NO_UNIQUE_ADDRESS TScheduler scheduler;
    schedulers::duration             delay;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto worker = scheduler.create_worker(subscriber.get_subscription());

        auto subscription = subscriber.get_subscription().make_child();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  delay_on_next{delay},
                                                  delay_on_error{},
                                                  delay_on_completed{delay},
                                                  std::forward<TSub>(subscriber),
                                                  std::move(worker));
    }
};
} // namespace rpp::details
