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
#include <thread>

namespace rpp::schedulers::details
{
// keep old_timepoint to easily understand if we need to sleep (due to sleep is expensive enough even if time in the "past")
inline thread_local time_point s_last_sleep_timepoint{};

/**
 * @brief Makes immediate-like scheduling for provided arguments
 * @returns false in case of subscription unsubscribed or schedulable doesn't requested to re-schedule, true - in case of condition failed
 **/
template<rpp::constraint::observer TObs, typename... Args>
bool immediate_scheduling_while_condition(time_point&                                      time_point,
                                          const std::predicate auto&                       condition,
                                          constraint::schedulable_fn<TObs, Args...> auto&& fn,
                                          TObs&&                                           obs,
                                          Args&&... args)
{
    while (condition())
    {
        if (obs.is_disposed())
            return false;

        if (s_last_sleep_timepoint < time_point)
        {
            std::this_thread::sleep_until(time_point);
            s_last_sleep_timepoint = time_point;

            if (obs.is_disposed())
                return false;
        }

        if (const auto duration = fn(obs, args...))
        {
            if (duration.value() != duration::zero())
                time_point = clock_type::now() + duration.value();
        }
        else
            return false;
    }

    return true;
}
} // namespace rpp::schedulers::details