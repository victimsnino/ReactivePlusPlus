//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/schedulers/details/queue.hpp>
#include <rpp/schedulers/details/utils.hpp>
#include <rpp/schedulers/details/worker.hpp>
#include <rpp/schedulers/fwd.hpp>

namespace rpp::schedulers
{
/**
 * @brief Schedules execution of schedulables via queueing tasks to the caller thread with priority to time_point and order.
 * @warning Caller thread is thread where "schedule" called.
 *
 * @par Example
 * @snippet current_thread.cpp current_thread
 *
 * @ingroup schedulers
 */
class current_thread
{
    inline static thread_local std::optional<details::none_lock_queue> s_queue{};

    static void drain_queue()
    {
        if (!s_queue.has_value())
            return;

        auto reset_at_final = utils::finally_action{[] { s_queue.reset(); }};

        while (!s_queue->is_empty())
            s_queue->dispatch();
    }

public:
    class worker_strategy
    {
    public:
        template<rpp::constraint::observer TObs, typename... Args, constraint::schedulable_fn<TObs, Args...> Fn>
        static void defer_for(duration duration, Fn&& fn, TObs&& obs, Args&&... args)
        {
            if (obs.is_disposed())
                return;

            const bool someone_owns_queue = s_queue.has_value();
            const auto drain_on_exit      = utils::finally_action(!someone_owns_queue ? &drain_queue : +[]{});
            if (!someone_owns_queue)
            {
                s_queue.emplace();

                const auto optional_duration = details::immediate_scheduling_while_condition(duration, [](){ return s_queue->is_empty(); }, fn, obs, args...);
                if (!optional_duration || obs.is_disposed())
                    return;
                duration = optional_duration.value();
            }

            s_queue->emplace(duration, std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...);
        }

        static rpp::disposable_wrapper get_disposable() { return rpp::disposable_wrapper{}; }
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }
};
} // namespace rpp::schedulers