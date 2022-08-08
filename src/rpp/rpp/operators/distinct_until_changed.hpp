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

#include <rpp/observables/constraints.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/operators/fwd/distinct_until_changed.hpp>
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/utils/functors.hpp>

#include <rpp/defs.hpp>

#include <rpp/utils/utilities.hpp>

#include <memory>
#include <optional>


IMPLEMENTATION_FILE (distinct_until_changed_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, std::equivalence_relation<Type, Type> EqualityFn>
class distinct_until_changed_on_next
{
public:
    distinct_until_changed_on_next(const EqualityFn& comparator) : m_equality_comparator{comparator} {}

    void operator()(auto&& new_value, const rpp::constraint::subscriber auto& sub) const
    {
        if (m_last_value.has_value() && m_equality_comparator(utils::as_const(m_last_value.value()),
                                                              utils::as_const(new_value)))
            return;

        m_last_value.emplace(new_value);
        sub.on_next(std::forward<decltype(new_value)>(new_value));
    }

private:
    RPP_NO_UNIQUE_ADDRESS EqualityFn m_equality_comparator;
    mutable std::optional<Type>      m_last_value{};
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
                                                          std::forward<TSub>(subscriber),
                                                          distinct_until_changed_on_next<Type, EqualityFn>{equality_comparator},
                                                          utils::forwarding_on_error{},
                                                          utils::forwarding_on_completed{});
};
} // namespace rpp::details
