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

#include <rpp/operators/fwd/take.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/utils/functors.hpp>

IMPLEMENTATION_FILE(take_tag);

namespace rpp::details
{
struct take_on_next
{
    take_on_next(size_t count) : m_count{ count } {}

    void operator()(auto&& value, const constraint::subscriber auto& subscriber) const
    {
        if (m_count > 0)
        {
            --(m_count);
            subscriber.on_next(std::forward<decltype(value)>(value));
        }

        if (m_count == 0)
            subscriber.on_completed();
    };

private:
    mutable size_t m_count;
};

template<constraint::decayed_type Type>
struct take_impl
{
    size_t count;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  std::forward<TSub>(subscriber),
                                                  take_on_next{count},
                                                  utils::forwarding_on_error{},
                                                  utils::forwarding_on_completed{})
            .as_dynamic(); // use as_dynamic to make shared_ptr instead of making shared_ptr for take state
    }
};
} // namespace rpp::details
