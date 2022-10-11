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

#include <rpp/observables/fwd.hpp>                         // fwd specific_observable
#include <rpp/defs.hpp>                                    // RPP_NO_UNIQUE_ADDRESS
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/lift.hpp>                      // own forwarding
#include <rpp/sources/create.hpp>

IMPLEMENTATION_FILE(lift_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::decayed_type OnNext, constraint::decayed_type OnError, constraint::decayed_type OnCompleted>
struct lift_action_by_callbacks
{
    RPP_NO_UNIQUE_ADDRESS OnNext      on_next;
    RPP_NO_UNIQUE_ADDRESS OnError     on_error;
    RPP_NO_UNIQUE_ADDRESS OnCompleted on_completed;

    template<constraint::subscriber TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  on_next,
                                                  on_error,
                                                  on_completed,
                                                  std::forward<TSub>(subscriber));
    }
};

/**
 * \brief Functor of "lift" operator for on_subscribe overload function.
 * \details Each observable has an on_subscribe function and observable is activated (pub-sub channel is established) after on_subscribe is called. The on_subscribe is called when the observable is subscribed by a subscriber
 *
 * \param _this is the current observable.
 * \param op is the functor that provides the "operator()(subscriber_of_new_type) -> subscriber_of_old_type".
 */
template<constraint::decayed_type NewType, lift_fn<NewType> OperatorFn, typename TObs>
struct lift_on_subscribe
{
    RPP_NO_UNIQUE_ADDRESS TObs _this;
    RPP_NO_UNIQUE_ADDRESS OperatorFn op;

    template<constraint::subscriber_of_type<NewType> TSub>
    void operator()(TSub&& subscriber) const
    {
        _this.subscribe(op(std::forward<TSub>(subscriber)));
    }
};

template<constraint::decayed_type NewType, lift_fn<NewType> OperatorFn, typename TObs>
auto lift_impl(OperatorFn&& op, TObs&& _this)
{
    return rpp::observable::create(lift_on_subscribe<NewType, std::decay_t<OperatorFn>, std::decay_t<TObs>>{ std::forward<TObs>(_this), std::forward<OperatorFn>(op) });
}
} // namespace rpp::details
