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
#include <rpp/operators/fwd/scan.hpp>
#include <rpp/observers/state_observer.hpp>

#include <memory>

IMPLEMENTATION_FILE (scan_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::decayed_type Result, scan_accumulator<Result, Type> AccumulatorFn>
struct scan_impl
{
    Result                              initial_value;
    [[no_unique_address]] AccumulatorFn accumulator;

    template<constraint::subscriber_of_type<Result> TSub>
    void operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<Result>(initial_value);

        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  std::move(subscriber),
                                                  [state, accumulator=accumulator](auto&& value, const auto& sub)
                                                  {
                                                      *state = accumulator(std::move(*state), std::forward<decltype(value)>(value));
                                                      sub.on_next(*state);
                                                  },
                                                  forwarding_on_error{},
                                                  forwarding_on_completed{});
    }
};
} // namespace rpp::details

