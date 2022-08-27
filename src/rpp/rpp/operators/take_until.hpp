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
#include <rpp/operators/first.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/functors.hpp>


#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state


IMPLEMENTATION_FILE(take_until_tag);

namespace rpp::details
{

/**
 * Functor (type-erasure) of "take_until" for on_next operator.
 */
template<constraint::decayed_type Type>
struct take_until_on_next
{
    void operator()(auto&& value,
                    const auto& subscriber,
                    const std::shared_ptr<first_state>& state) const
    {
        if (state->count > 0)
            subscriber.on_next(std::forward<decltype(value)>(value));
        else
            subscriber.on_completed();
    }
};

template<constraint::decayed_type Type>
struct take_until_throttler
{

    /**
     * Functor (type-erasure) of "take_until" for on_next operator.
     */
    void operator()(auto&&,
                    const auto& subscriber,
                    const std::shared_ptr<first_state>& state) const
    {
        if (state->count > 0)
            --state->count;

        if (state->count == 0)
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
        auto state = std::make_shared<first_state>();

        // Subscribe to trigger observable
        auto child_subscription = subscriber.get_subscription().make_child();
        m_until_observable.subscribe(
            create_subscriber_with_state<TriggerType>(std::move(child_subscription),
                                                      take_until_throttler<TriggerType>{},
                                                      utils::forwarding_on_error{},
                                                      utils::forwarding_on_completed{},
                                                      std::forward<decltype(subscriber)>(subscriber),
                                                      state));

        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  take_until_on_next<Type>{},
                                                  utils::forwarding_on_error{},
                                                  utils::forwarding_on_completed{},
                                                  std::forward<decltype(subscriber)>(subscriber),
                                                  std::move(state));
    }
};

} // namespace rpp::details
