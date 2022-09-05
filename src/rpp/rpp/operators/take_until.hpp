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
#include <rpp/operators/fwd/take_until.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/functors.hpp>

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state

#include <mutex>

IMPLEMENTATION_FILE(take_until_tag);

namespace rpp::details
{

struct take_until_state
{
    std::mutex mutex;
    bool is_stopped{false};
};

/**
 * Functor (type-erasure) of "take_until" for on_next operator.
 */
template<constraint::decayed_type Type>
struct take_until_on_next
{
    void operator()(auto &&value,
                    const auto &subscriber,
                    const std::shared_ptr<take_until_state> &state) const {
        std::lock_guard lock{state->mutex};

        if (!state->is_stopped)
            subscriber.on_next(std::forward<decltype(value)>(value));
    }
};

/**
 * Functor (type-erasure) of "take_until" for on_error operator.
 */
struct take_until_on_error
{
    void operator()(const std::exception_ptr &err,
                    const auto &subscriber,
                    const std::shared_ptr<take_until_state> &state) const
    {
        std::lock_guard lock{state->mutex};

        subscriber.on_error(err);
    }
};

/**
 * Functor (type-erasure) of "take_until" for on_completed operator.
 */
struct take_until_on_completed
{
    void operator()(const auto& subscriber,
                    const std::shared_ptr<take_until_state>& state) const
    {
        std::lock_guard lock{state->mutex};

        subscriber.on_completed();
    }
};

/**
 * Functor (type-erasure) of throttler (trigger observable) for on_next operator.
 */
template<constraint::decayed_type Type>
struct take_until_throttler_on_next {
    void operator()(auto &&,
                    const auto &subscriber,
                    const std::shared_ptr<take_until_state> &state) const
    {
        std::lock_guard lock{state->mutex};

        state->is_stopped = true;
        subscriber.on_completed();
    }
};

/**
 * Functor (type-erasure) of throttler (trigger observable) for on_error operator.
 */
struct take_until_throttler_on_error {
    void operator()(const std::exception_ptr &err,
                    const auto &subscriber,
                    const std::shared_ptr<take_until_state> &state) const
    {
        std::lock_guard lock{state->mutex};

        subscriber.on_error(err);
    }
};

/**
 * Functor (type-erasure) of throttler (trigger observable) for on_completed operator.
 */
struct take_until_throttler_on_completed
{
    void operator()(const auto& subscriber,
                    const std::shared_ptr<take_until_state>& state) const
    {
        std::lock_guard lock{state->mutex};

        state->is_stopped = true;
        subscriber.on_completed();
    }
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
    auto operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<take_until_state>();

        // Subscribe to trigger observable
        auto child_subscription = subscriber.get_subscription().make_child();
        m_until_observable.subscribe(
            create_subscriber_with_state<TriggerType>(std::move(child_subscription),
                                                      take_until_throttler_on_next<TriggerType>{},
                                                      take_until_throttler_on_error{},
                                                      take_until_throttler_on_completed{},
                                                      std::forward<decltype(subscriber)>(subscriber),
                                                      state));

        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  take_until_on_next<Type>{},
                                                  take_until_on_error{},
                                                  take_until_on_completed{},
                                                  std::forward<decltype(subscriber)>(subscriber),
                                                  std::move(state));
    }
};

} // namespace rpp::details
