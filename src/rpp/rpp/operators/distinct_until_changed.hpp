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
#include <rpp/observers/state_observer.hpp>

#include <memory>
#include <optional>


IMPLEMENTATION_FILE (distinct_until_changed_tag);

namespace rpp::details
{
template<constraint::decayed_type Type>
struct distinct_until_changed_impl
{
    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();

        auto shared_optional = std::make_shared<std::optional<Type>>();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  std::forward<TSub>(subscriber),
                                                  [shared_optional](auto&& new_value, const auto& sub)
                                                  {
                                                      if (*shared_optional == new_value)
                                                          return;
                                                      shared_optional->emplace(new_value);
                                                      sub.on_next(std::forward<decltype(new_value)>(new_value));
                                                  },
                                                  forwarding_on_error{},
                                                  forwarding_on_completed{});
    }
};
} // namespace rpp::details
