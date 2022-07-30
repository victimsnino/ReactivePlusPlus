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

#include <algorithm>

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/buffer.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/sources/create.hpp>

IMPLEMENTATION_FILE(buffer_tag);

namespace rpp::details
{

/// A non-copyable class that provides a copyable on_next for the subscriber and
/// allows copies of on_next(s) to share the same states.
template<constraint::decayed_type UpstreamType, constraint::decayed_type DownstreamType>
struct buffer_on_next : public std::enable_shared_from_this<buffer_on_next<UpstreamType, DownstreamType>>
{
    using DownstreamValueType = UpstreamType;

    /// \param count Number of items being bundled. Note when count == 0, we'll
    /// treat the behavior like when count == 1.
    explicit buffer_on_next(size_t count) : max_(std::max<size_t>(1ul, count)) {
        reserve_buckets();
    };

    // Copy & move constructors
    buffer_on_next(const buffer_on_next& other) = delete;
    buffer_on_next(buffer_on_next&&) noexcept = default;

    // Copy & move assignment operators
    buffer_on_next& operator=(const buffer_on_next&) = delete;
    buffer_on_next& operator=(buffer_on_next&&) noexcept = default;

    /// \return a on_next function for subscriber, that is copyable and all the
    /// copies share the same state.
    auto get_on_next() {
        return [shared_this = this->shared_from_this()](
                auto&& value,
                const constraint::subscriber_of_type<DownstreamType> auto& subscriber)
        {
            shared_this->buckets_.push_back(std::forward<decltype(value)>(value));
            if (shared_this->buckets_.size() == shared_this->max_)
            {
                subscriber.on_next(std::move(shared_this->buckets_));
                shared_this->reserve_buckets();
            }
        };
    }

    /// \return a on_completed function for subscriber, that is copyable and all the
    /// copies share the same state.
    auto get_on_completed() {
        return [shared_this = this->shared_from_this()](
                const constraint::subscriber_of_type<DownstreamType> auto& subscriber)
        {
            if (!shared_this->buckets_.empty())
            {
                subscriber.on_next(std::move(shared_this->buckets_));
            }
            subscriber.on_completed();
        };
    }

private:
    void reserve_buckets() {
        buckets_.reserve(max_);
    }

private:
    const size_t max_;
    std::vector<DownstreamValueType> buckets_;
};

template<constraint::decayed_type UpstreamType, constraint::decayed_type DownstreamType>
struct buffer_impl
{
    const size_t count;

    template<constraint::subscriber_of_type<DownstreamType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<buffer_on_next<UpstreamType, DownstreamType>>(count);
        auto subscription = subscriber.get_subscription();

        return create_subscriber_with_state<UpstreamType>(
            std::move(subscription),
            std::forward<TSub>(subscriber),
            // Get a copy of on_next that shares the same state.
            state->get_on_next(),
            forwarding_on_error{},
            // Flush the bundle buffer on complete.
            state->get_on_completed());
    }
};

} // namespace rpp::details
