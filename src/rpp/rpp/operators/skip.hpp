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

#include <rpp/operators/fwd/skip.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/observers/state_observer.hpp>

#include <memory>

IMPLEMENTATION_FILE(skip_tag);

namespace rpp::details
{
    struct skip_on_next
    {
        skip_on_next(size_t count) : m_shared_count{ std::make_shared<size_t>(count) } {}

        void operator()(auto&& value, const constraint::subscriber auto& subscriber) const
        {
            if (*m_shared_count == 0)
                subscriber.on_next(std::forward<decltype(value)>(value));
            else
                --(*m_shared_count);
        };

    private:
        std::shared_ptr<size_t> m_shared_count;
    };

    template<constraint::decayed_type Type>
    struct skip_impl
    {
        size_t count;

        template<constraint::subscriber_of_type<Type> TSub>
        auto operator()(TSub&& subscriber) const
        {
            auto subscription = subscriber.get_subscription();
            return create_subscriber_with_state<Type>(std::move(subscription), std::forward<TSub>(subscriber), skip_on_next{ count }, forwarding_on_error{}, forwarding_on_completed{});
        }
    };
} // namespace rpp::details
