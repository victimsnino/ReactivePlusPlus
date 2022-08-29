//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                            TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/operators/fwd/first.hpp>
#include <rpp/operators/take.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/exceptions.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/utils/utilities.hpp>

#include <memory>

IMPLEMENTATION_FILE(first_tag);

namespace rpp::details
{
struct first_state : take_state
{
    first_state()
        : take_state{1} {}
};

using first_on_next = take_on_next;

struct first_on_completed
{
    void operator()(const constraint::subscriber auto& subscriber, const first_state& state) const
    {
        if (state.count != 0)
            subscriber.on_error(std::make_exception_ptr(utils::not_enough_emissions{"first() operator expects at least one emission from observable before completion"}));
    }
};

template<constraint::decayed_type Type>
struct first_impl
{
public:
    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();

        // dynamic_state there to make shared_ptr for observer instead of making shared_ptr for state
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  first_on_next{},
                                                  utils::forwarding_on_error{},
                                                  first_on_completed{},
                                                  std::forward<TSub>(subscriber),
                                                  first_state{});
    }
};
} // namespace rpp::details
