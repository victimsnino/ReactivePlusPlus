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

#include <rpp/operators/fwd/last.hpp>
#include <rpp/operators/take_last.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/exceptions.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/utils/utilities.hpp>

IMPLEMENTATION_FILE(last_tag);

namespace rpp::details
{

template<constraint::decayed_type Type>
struct last_state : public take_last_state<Type>
{
    explicit last_state() : take_last_state<Type>{1} {}
};

/**
 * Functor of last() operator for on_next events.
 */
using last_on_next = take_last_on_next;

/**
 * Functor of last() operator for on_completed event.
 */
struct last_on_completed
{
    template<constraint::decayed_type Type>
    void operator()(const constraint::subscriber auto& subscriber,
                    const last_state<Type>& state) const
    {
        auto&& last_value = state.items.at(0);
        if (!last_value.has_value())
        {
            subscriber.on_error(std::make_exception_ptr(utils::not_enough_emissions{"last() operator expects at least one emission from observable before completion"}));
            return;
        }

        subscriber.on_next(std::move(last_value.value()));
        subscriber.on_completed();
    }
};

template<constraint::decayed_type Type>
struct last_impl
{
public:
    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();

        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  last_on_next{},
                                                  utils::forwarding_on_error{},
                                                  last_on_completed{},
                                                  std::forward<TSub>(subscriber),
                                                  last_state<Type>{});
    }
};
} // namespace rpp::details
