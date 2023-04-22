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
#include <thread>

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
    inline static thread_local std::optional<std::priority_queue<details::schedulable>> s_queue{};
    inline static thread_local size_t s_counter{};

    static void sleep_until(const time_point timepoint)
    {
        static thread_local time_point s_sleep_in_this_thread{};
        if (timepoint <= s_sleep_in_this_thread)
            return;

        std::this_thread::sleep_until(timepoint);
        s_sleep_in_this_thread = timepoint;
    }

    static void drain_queue(std::optional<std::priority_queue<details::schedulable>>& queue)
    {
        if (!queue.has_value())
            return;

        while (!queue->empty())
        {
            if (queue->top().get_function().is_disposed())
            {
                queue->pop();
                continue;
            }

            sleep_until(queue->top().get_time_point());

            auto function = queue->top().get_function();
            queue->pop();

            optional_duration duration{0};
            // immediate like scheduling
            do
            {
                if (duration.value() > duration::zero() && !function.is_disposed())
                    std::this_thread::sleep_for(duration.value());

                if (function.is_disposed())
                    duration.reset();
                else
                    duration = function();

            } while (queue->empty() && duration.has_value());

            if (duration.has_value())
                queue->emplace(clock_type::now() + duration.value(), s_counter++, std::move(function));
        }

        queue.reset();
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

            auto& queue = s_queue;
            const bool someone_owns_queue = queue.has_value();
            if (!someone_owns_queue)
            {
                queue.emplace();

                const auto optional_duration = details::immediate_scheduling_while_condition(duration, [&queue](){ return queue->empty(); }, fn, obs, args...);
                if (!optional_duration || obs.is_disposed())
                    return drain_queue(queue);
                duration = optional_duration.value();
            }

            queue->emplace(clock_type::now() + duration, s_counter++, details::schedulable_wrapper{std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...});

            if (!someone_owns_queue)
                drain_queue(queue);
        }

        static rpp::disposable_wrapper get_disposable() { return rpp::disposable_wrapper{}; }
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }
};
} // namespace rpp::schedulers