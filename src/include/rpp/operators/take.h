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

#include <rpp/operators/fwd/take.h>
#include <rpp/observables/constraints.h>
#include <rpp/subscribers/constraints.h>

#include <atomic>
#include <memory>

IMPLEMENTATION_FILE(take_tag);

namespace rpp::operators
{
template<typename...Args>
auto take(size_t count) requires details::is_header_included<details::take_tag, Args...>
{
    return [count]<constraint::observable TObservable>(TObservable&& observable)
    {
        return observable.take(count);
    };
}
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
auto member_overload<Type, SpecificObservable, take_tag>::take_impl(size_t count)
{
    struct state
    {
        state(size_t count) : count{count} {}
        
        const size_t       count;
        std::atomic_size_t sent_count{};
    };

    return [count]<constraint::subscriber_of_type<Type> TSub>(TSub&& subscriber)
    {
        auto action = [state = std::make_shared<state>(count)](auto&& value, const constraint::subscriber_of_type<Type> auto& subscriber)
        {
            const auto old_value = state->sent_count.fetch_add(1);
            if (old_value < state->count)
            {
                subscriber.on_next(std::forward<decltype(value)>(value));
                if (state->count - old_value == 1)
                    subscriber.on_completed();
            }
        };

        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(subscription, std::forward<TSub>(subscriber), std::move(action), forwarding_on_error{}, forwarding_on_completed{});
    };
}
} // namespace rpp::details
