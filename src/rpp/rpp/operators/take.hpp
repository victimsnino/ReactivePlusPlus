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
#include <rpp/observers/state_observer.hpp>

#include <memory>

IMPLEMENTATION_FILE(take_tag);

namespace rpp::details
{
template<constraint::decayed_type Type>
struct take_impl
{
    size_t count;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(subscription), std::forward<TSub>(subscriber), make_action(), forwarding_on_error{}, forwarding_on_completed{});
    }

private:
    auto make_action() const
    {
        return [shared_count = std::make_shared<size_t>(count)](auto&& value, const constraint::subscriber_of_type<Type> auto& subscriber)
        {
            if (*shared_count > 0)
            {
                --(*shared_count);
                subscriber.on_next(std::forward<decltype(value)>(value));
            }

            if (*shared_count == 0)
                subscriber.on_completed();
        };
    }
};
} // namespace rpp::details
