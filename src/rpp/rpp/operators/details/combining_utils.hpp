//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subscribers/constraints.hpp>
#include <rpp/subscriptions/composite_subscription.hpp>

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state

#include <atomic>

namespace rpp::details::combining
{
template<constraint::decayed_type Type>
auto create_proxy_subscriber(rpp::composite_subscription subscription,
                             constraint::subscriber auto&& subscriber,
                             std::atomic_size_t& count_of_on_completed_required,
                             auto&& on_next,
                             auto&& on_error,
                             auto&& on_completed)
{
    // we don't need to add some memory barrier there
    count_of_on_completed_required.fetch_add(1, std::memory_order::relaxed);

    return create_subscriber_with_state<Type>(std::move(subscription),
                                              std::forward<decltype(subscriber)>(subscriber),
                                              std::forward<decltype(on_next)>(on_next),
                                              std::forward<decltype(on_error)>(on_error),
                                              std::forward<decltype(on_completed)>(on_completed));
}

template<constraint::decayed_type Type>
auto create_proxy_subscriber(constraint::subscriber auto&& subscriber,
                             std::atomic_size_t& count_of_on_completed_required,
                             auto&& on_next,
                             auto&& on_error,
                             auto&& on_completed)
{
    auto sub = subscriber.get_subscription().make_child();
    return create_proxy_subscriber<Type>(std::move(sub),
                                         std::forward<decltype(subscriber)>(subscriber),
                                         count_of_on_completed_required,
                                         std::forward<decltype(on_next)>(on_next),
                                         std::forward<decltype(on_error)>(on_error),
                                         std::forward<decltype(on_completed)>(on_completed));
}
} // namespace rpp::details::combining
