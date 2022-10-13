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

#include <rpp/defs.hpp>                                    // RPP_NO_UNIQUE_ADDRESS
#include <rpp/operators/lift.hpp>                          // required due to operator uses lift
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_dynamic_state
#include <rpp/operators/fwd/distinct_until_changed.hpp>    // own forwarding
#include <rpp/subscribers/constraints.hpp>                 // constraint::subscriber_of_type
#include <rpp/utils/functors.hpp>                          // forwarding_on_error/forwarding_on_completed
#include <rpp/utils/utilities.hpp>                         // as_const

#include <optional>


IMPLEMENTATION_FILE(distinct_until_changed_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, std::equivalence_relation<Type, Type> EqualityFn>
struct distinct_until_changed_state
{
    RPP_NO_UNIQUE_ADDRESS EqualityFn equality_comparator;
    mutable std::optional<Type>      last_value{};
};

struct distinct_until_changed_on_next
{
    template<constraint::decayed_type Type, std::equivalence_relation<Type, Type> EqualityFn>
    void operator()(auto&&                                                new_value,
                    const constraint::subscriber auto&                    sub,
                    const distinct_until_changed_state<Type, EqualityFn>& state) const
    {
        if (state.last_value.has_value() &&
            state.equality_comparator(utils::as_const(state.last_value.value()),
                                      utils::as_const(new_value)))
            return;

        state.last_value.emplace(new_value);
        sub.on_next(std::forward<decltype(new_value)>(new_value));
    }
};

template<constraint::decayed_type Type, std::equivalence_relation<Type, Type> EqualityFn>
struct distinct_until_changed_impl
{
    RPP_NO_UNIQUE_ADDRESS EqualityFn equality_comparator;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        // dynamic_state there to make shared_ptr for observer instead of making shared_ptr for state
        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          distinct_until_changed_on_next{},
                                                          utils::forwarding_on_error{},
                                                          utils::forwarding_on_completed{},
                                                          std::forward<TSub>(subscriber),
                                                          distinct_until_changed_state<Type, EqualityFn>{equality_comparator});
    }
};
} // namespace rpp::details
