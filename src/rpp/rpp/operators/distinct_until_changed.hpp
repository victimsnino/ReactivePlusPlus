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

#include <rpp/utils/utilities.hpp>

#include <memory>
#include <optional>


IMPLEMENTATION_FILE (distinct_until_changed_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, std::equivalence_relation<Type, Type> EqualityFn>
struct distinct_until_changed_impl
{
    [[no_unique_address]] EqualityFn equality_comparator;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();

        auto shared_optional = std::make_shared<std::optional<Type>>();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  std::forward<TSub>(subscriber),
                                                  [shared_optional, equality_comparator=equality_comparator](auto&& new_value, const auto& sub)
                                                  {
                                                      if (!shared_optional->has_value() || !equality_comparator(utils::as_const(shared_optional->value()), utils::as_const(new_value)))
                                                      {
                                                          shared_optional->emplace(new_value);

                                                          sub.on_next(std::forward<decltype(new_value)>(new_value));
                                                      }
                                                  },
                                                  forwarding_on_error{},
                                                  forwarding_on_completed{});
    }
};
} // namespace rpp::details
