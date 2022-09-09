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
#include <rpp/operators/fwd/delay.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/functors.hpp>

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state

IMPLEMENTATION_FILE(delay_tag);

namespace rpp::details
{

/**
 * Functor (type-erasure) of "delay" for on_next operator.
 */
struct delay_on_next
{
    rpp::schedulers::duration delay;

    void operator()(auto &&value,
                    const auto &subscriber,
                    const auto &worker) const {
        worker.schedule(delay,
                        [value = std::forward<decltype(value)>(value), subscriber]() -> rpp::schedulers::optional_duration
                        {
                            subscriber.on_next(std::move(value));
                            return std::nullopt;
                        });
    }
};

/**
 * Functor (type-erasure) of "delay" for on_error operator.
 */
struct delay_on_error
{
    rpp::schedulers::duration delay;

    void operator()(const std::exception_ptr &err,
                    const auto &subscriber,
                    const auto& worker) const
    {
        worker.schedule(delay,
                        [err, subscriber]() -> rpp::schedulers::optional_duration
                        {
                            subscriber.on_error(err);
                            return std::nullopt;
                        });
    }
};

/**
 * Functor (type-erasure) of "delay" for on_completed operator.
 */
struct delay_on_completed
{
    rpp::schedulers::duration delay;

    void operator()(const auto& subscriber,
                    const auto& worker) const
    {
        worker.schedule(delay,
                        [subscriber]() -> rpp::schedulers::optional_duration
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
    rpp::schedulers::duration delay;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        // convert it to dynamic due to expected amount of copies == amount of items
        auto dynamic_subscriber = std::forward<TSub>(subscriber).as_dynamic();
        auto subscription = dynamic_subscriber.get_subscription().make_child();

        auto worker = scheduler.create_worker(dynamic_subscriber.get_subscription());

        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  delay_on_next{delay},
                                                  delay_on_error{delay},
                                                  delay_on_completed{delay},
                                                  dynamic_subscriber,
                                                  std::move(worker));
    }
};

} // namespace rpp::details
