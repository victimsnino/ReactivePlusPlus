//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/schedulers/fwd.hpp>
#include <optional>
#include <thread>

namespace rpp::schedulers::details
{
/**
 * @brief Makes immediate-like scheduling for provided arguments
 * @returns nullopt in case of subscription unsubscribed or schedulable doesn't requested to re-schedule, some value - in case of condition failed but still some duration to delay action
 **/
template<rpp::constraint::observer TObs, typename... Args>
optional_duration immediate_scheduling_while_condition(duration                                         duration,
                                                       const std::predicate auto&                       condition,
                                                       constraint::schedulable_fn<TObs, Args...> auto&& fn,
                                                       TObs&&                                           obs,
                                                       Args&&... args)
{
    while (condition())
    {
        if (obs.is_disposed())
            return std::nullopt;

        if (duration > duration::zero())
        {
            std::this_thread::sleep_for(duration);

            if (obs.is_disposed())
                return std::nullopt;
        }

        if (const auto new_duration = fn(obs, args...))
            duration = new_duration.value();
        else
            return std::nullopt;
    }

    return duration;
}
} // namespace rpp::schedulers::details