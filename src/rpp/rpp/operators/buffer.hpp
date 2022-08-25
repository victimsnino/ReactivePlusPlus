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
struct buffer_on_next : public std::enable_shared_from_this<buffer_on_next<UpstreamType>>
{
    /// \param count Number of items being bundled. Note when count == 0, we'll
    /// treat the behavior like when count == 1.
    explicit buffer_on_next(size_t count)
        : m_max(std::max(size_t{1}, count))
    {
        reserve_buckets();
    };

    // Copy & move constructors
    buffer_on_next(const buffer_on_next& other) = delete;
    buffer_on_next(buffer_on_next&&) noexcept   = default;

    // Copy & move assignment operators
    buffer_on_next& operator=(const buffer_on_next&)     = delete;
    buffer_on_next& operator=(buffer_on_next&&) noexcept = default;

    /// \return a on_next function for subscriber, that is copyable and all the
    /// copies share the same state.
    auto get_on_next()
    {
        return [shared_this = this->shared_from_this()](auto&& value, const auto& subscriber)
        {
            shared_this->m_buckets.push_back(std::forward<decltype(value)>(value));
            if (shared_this->m_buckets.size() == shared_this->m_max)
            {
                subscriber.on_next(std::move(shared_this->m_buckets));
                shared_this->reserve_buckets();
            }
        };
    }

    /// \return a on_completed function for subscriber, that is copyable and all the
    /// copies share the same state.
    auto get_on_completed()
    {
        return [shared_this = this->shared_from_this()](const auto& subscriber)
        {
            if (!shared_this->m_buckets.empty())
            {
                subscriber.on_next(std::move(shared_this->m_buckets));
            }
            subscriber.on_completed();
        };
    }

private:
    void reserve_buckets()
    {
        m_buckets.clear();
        m_buckets.reserve(m_max);
    }

private:
    const size_t                     m_max;
    buffer_bundle_type<UpstreamType> m_buckets;
};

template<constraint::decayed_type Type>
struct buffer_impl
{
    const size_t count;

    template<constraint::subscriber_of_type<buffer_bundle_type<Type>> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<buffer_on_next<Type>>(count);
        auto subscription = subscriber.get_subscription();

        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  // Get a copy of on_next that shares the same state.
                                                  state->get_on_next(),
                                                  // Get a copy of on_next that shares the same state.
                                                  utils::forwarding_on_error{},
                                                  // Flush the bundle buffer on complete.
                                                  state->get_on_completed(),
                                                  // Flush the bundle buffer on complete.
                                                  std::forward<TSub>(subscriber));
    }
};

} // namespace rpp::details
