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

#include <rpp/operators/fwd/buffer.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/utils/functors.hpp>

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state


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
        : m_max(std::max(size_t{1}, count))
    {
        reserve_buckets();
    }

    buffer_state(const buffer_state& other)          = delete;
    buffer_state(buffer_state&&) noexcept            = default;
    buffer_state& operator=(const buffer_state&)     = delete;
    buffer_state& operator=(buffer_state&&) noexcept = default;

    struct on_next
    {
        void operator()(auto&& value, const auto& subscriber, const buffer_state<UpstreamType>& state) const
        {
            state.m_buckets.push_back(std::forward<decltype(value)>(value));
            if (state.m_buckets.size() == state.m_max)
            {
                subscriber.on_next(std::move(state.m_buckets));
                state.reserve_buckets();
            }
        }
    };

    struct on_completed
    {
        void operator()(const auto& subscriber, const buffer_state<UpstreamType>& state) const
        {
            if (!state.m_buckets.empty())
                subscriber.on_next(std::move(state.m_buckets));
            subscriber.on_completed();
        }
    };

private:
    void reserve_buckets() const
    {
        m_buckets.clear();
        m_buckets.reserve(m_max);
    }

private:
    const size_t                             m_max;
    mutable buffer_bundle_type<UpstreamType> m_buckets;
};

template<constraint::decayed_type Type>
struct buffer_impl
{
    const size_t count;

    template<constraint::subscriber_of_type<buffer_bundle_type<Type>> TSub>
    auto operator()(TSub&& subscriber) const
    {
        using state = buffer_state<Type>;

        auto subscription = subscriber.get_subscription();

        // dynamic_state there to make shared_ptr for observer instead of making shared_ptr for state
        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          typename state::on_next{},
                                                          utils::forwarding_on_error{},
                                                          typename state::on_completed{},
                                                          std::forward<TSub>(subscriber),
                                                          state{count});
    }
};

} // namespace rpp::details
