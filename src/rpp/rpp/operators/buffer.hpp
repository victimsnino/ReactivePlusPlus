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

#include <rpp/operators/lift.hpp>                          // required due to operator uses lift
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_dynamic_state
#include <rpp/operators/fwd/buffer.hpp>                    // own forwarding
#include <rpp/subscribers/constraints.hpp>                 // subscriber_of_type
#include <rpp/utils/functors.hpp>                          // forwarding_on_error

#include <algorithm>


IMPLEMENTATION_FILE(buffer_tag);

namespace rpp::details
{
/// A non-copyable class that provides a copyable on_next for the subscriber and
/// allows copies of on_next(s) to share the same states.
template<constraint::decayed_type UpstreamType, constraint::subscriber_of_type<UpstreamType> Subscriber>
struct buffer_state
{
    template<constraint::decayed_same_as<Subscriber> TSub>
    explicit buffer_state(size_t count, TSub&& sub)
        : max(std::max(size_t{1}, count))
        , subscriber{std::forward<TSub>(sub)}
    {
        clear_and_reserve_buckets();
    }

    buffer_state(const buffer_state& other)          = delete;
    buffer_state(buffer_state&&) noexcept            = delete;

    void clear_and_reserve_buckets() const
    {
        buckets.clear();
        buckets.reserve(max);
    }

    const size_t                     max;
    buffer_bundle_type<UpstreamType> buckets;
    Subscriber                       subscriber;
};

struct buffer_on_next
{
    template<typename T, constraint::decayed_type UpstreamType, constraint::subscriber_of_type<UpstreamType> Subscriber>
    void operator()(T&& value, const std::shared_ptr<buffer_state<UpstreamType, Subscriber>>& state) const
    {
        state->buckets.push_back(std::forward<T>(value));
        if (state.buckets.size() == state.max)
        {
            state->subscriber.on_next(std::move(state.buckets));
            state->clear_and_reserve_buckets();
        }
    }
};

struct buffer_on_completed
{
    template<constraint::decayed_type UpstreamType, constraint::subscriber_of_type<UpstreamType> Subscriber>
    void operator()(const buffer_state<UpstreamType, Subscriber>& state) const
    {
        if (!state.buckets.empty())
            state->subscriber.on_next(std::move(state.buckets));
        state->subscriber.on_completed();
    }
};

template<constraint::decayed_type Type>
struct buffer_impl
{
    const size_t count;

    template<constraint::subscriber_of_type<buffer_bundle_type<Type>> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        auto state        = std::make_shared<buffer_state<Type, std::decay_t<TSub>>>(count, std::forward<TSub>(subscriber));

        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  buffer_on_next{},
                                                  utils::forwarding_on_error{},
                                                  buffer_on_completed{},
                                                  std::move(state));
    }
};

} // namespace rpp::details
