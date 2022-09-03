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

#include <rpp/subscribers/constraints.hpp>
#include <rpp/operators/fwd/take_last.hpp>

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/utils/functors.hpp>

#include <optional>
#include <vector>

IMPLEMENTATION_FILE(take_last_tag);

namespace rpp::details
{
template<constraint::decayed_type Type>
struct take_last_state
{
    take_last_state(size_t count)
        : items(count) {}

    size_t get_next_position(size_t pos) const
    {
        return (pos + 1) % items.size();
    }

    mutable std::vector<std::optional<Type>> items;
    mutable size_t                           current_end_position{};
};

struct take_last_on_next
{
    template<typename T, constraint::decayed_type Type>
    void operator()(T&& v, const auto&, const take_last_state<Type>& state) const
    {
        // handle case "count==0"
        if (state.items.empty())
            return;

        state.items[state.current_end_position].emplace(std::forward<T>(v));
        state.current_end_position = state.get_next_position(state.current_end_position);
    }
};

struct take_last_on_completed
{
    template<constraint::decayed_type Type>
    void operator()(const auto& subscriber, const take_last_state<Type>& state) const
    {
        if (!state.items.empty())
        {
            size_t cur_pos = state.current_end_position;

            do
            {
                if (auto&& value = state.items[cur_pos])
                    subscriber.on_next(std::move(value.value()));

                cur_pos = state.get_next_position(cur_pos);

            } while (cur_pos != state.current_end_position);
        }
        subscriber.on_completed();
    }
};


template<constraint::decayed_type Type>
struct take_last_impl
{
    size_t count;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        // dynamic_state there to make shared_ptr for observer instead of making
        // shared_ptr for state
        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          take_last_on_next{},
                                                          utils::forwarding_on_error{},
                                                          take_last_on_completed{},
                                                          std::forward<TSub>(subscriber),
                                                          take_last_state<Type>{count});
    }
};
} // namespace rpp::details
