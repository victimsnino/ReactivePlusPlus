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
template<constraint::decayed_type UpstreamType>
struct buffer_state
{
    /// \param count Number of items being bundled. Note when count == 0, we'll
    /// treat the behavior like when count == 1.
    explicit buffer_state(size_t count)
        : max(std::max(size_t{1}, count))
    {
        clear_and_reserve_buckets();
    }

    buffer_state(const buffer_state& other)          = delete;
    buffer_state(buffer_state&&) noexcept            = default;
    buffer_state& operator=(const buffer_state&)     = delete;
    buffer_state& operator=(buffer_state&&) noexcept = default;

    void clear_and_reserve_buckets() const
    {
        buckets.clear();
        buckets.reserve(max);
    }

    const size_t                             max;
    mutable buffer_bundle_type<UpstreamType> buckets;
};

struct buffer_on_next
{
    template<constraint::decayed_type UpstreamType>
    void operator()(auto&& value, const auto& subscriber, const buffer_state<UpstreamType>& state) const
    {
        state.buckets.push_back(std::forward<decltype(value)>(value));
        if (state.buckets.size() == state.max)
        {
            subscriber.on_next(std::move(state.buckets));
            state.clear_and_reserve_buckets();
        }
    }
};

struct buffer_on_completed
{
    template<constraint::decayed_type UpstreamType>
    void operator()(const auto& subscriber, const buffer_state<UpstreamType>& state) const
    {
        if (!state.buckets.empty())
            subscriber.on_next(std::move(state.buckets));
        subscriber.on_completed();
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

        // dynamic_state there to make shared_ptr for observer instead of making shared_ptr for state
        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          buffer_on_next{},
                                                          utils::forwarding_on_error{},
                                                          buffer_on_completed{},
                                                          std::forward<TSub>(subscriber),
                                                          buffer_state<Type>{count});
    }
};

} // namespace rpp::details
