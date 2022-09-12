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

#include <rpp/defs.hpp>
#include <rpp/operators/delay.hpp>
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/observe_on.hpp>
#include <rpp/subscribers/constraints.hpp>


IMPLEMENTATION_FILE(observe_on_tag);


namespace rpp::details
{
using observe_on_on_next = delay_on_next;
using observe_on_on_error = delay_on_error;
using observe_on_on_completed = delay_on_completed;

template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct observe_on_impl
{
    RPP_NO_UNIQUE_ADDRESS TScheduler scheduler;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        // convert it to dynamic due to expected amount of copies == amount of items
        auto dynamic_subscriber = std::forward<TSub>(subscriber).as_dynamic();

        auto worker = scheduler.create_worker(dynamic_subscriber.get_subscription());

        return create_subscriber_with_state<Type>(dynamic_subscriber.get_subscription().make_child(),
                                                  observe_on_on_next{rpp::schedulers::duration{0}},
                                                  observe_on_on_error{rpp::schedulers::duration{0}},
                                                  observe_on_on_completed{rpp::schedulers::duration{0}},
                                                  std::move(dynamic_subscriber),
                                                  std::move(worker));
    }
};
} // namespace rpp::details
