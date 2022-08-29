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

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/skip.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/subscribers/dynamic_subscriber.hpp>
#include <rpp/utils/functors.hpp>

IMPLEMENTATION_FILE(skip_tag);

namespace rpp::details
{
struct skip_state
{
    mutable size_t count;
};

struct skip_on_next
{
    void operator()(auto&& value, const constraint::subscriber auto& subscriber, const skip_state& state) const
    {
        if (state.count == 0)
            subscriber.on_next(std::forward<decltype(value)>(value));
        else
            --state.count;
    }
};

template<constraint::decayed_type Type>
struct skip_impl
{
    size_t count;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        // dynamic_state there to make shared_ptr for observer instead of making shared_ptr for state
        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          skip_on_next{},
                                                          utils::forwarding_on_error{},
                                                          utils::forwarding_on_completed{},
                                                          std::forward<TSub>(subscriber),
                                                          skip_state{count});
    }
};
} // namespace rpp::details
