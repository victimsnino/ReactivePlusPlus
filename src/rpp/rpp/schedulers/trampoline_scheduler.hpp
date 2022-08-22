//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                            TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once


#include <rpp/schedulers/fwd.hpp>                       // own forwarding
#include <rpp/schedulers/details/worker.hpp>            // worker
#include <rpp/subscriptions/composite_subscription.hpp> // lifetime
#include <rpp/schedulers/details/queue_worker_state.hpp>// state
#include <rpp/utils/utilities.hpp>
#include <rpp/schedulers/details/utils.hpp>

#include <concepts>
#include <chrono>
#include <functional>
#include <thread>


namespace rpp::schedulers
{
/**
 * \brief schedules execution of schedulables via queueing tasks to the caller thread with priority to time_point and order
 *
 * \par Example
 * \snippet trampoline.cpp trampoline
 *
 * \ingroup schedulers
 */
class trampoline final : public details::scheduler_tag
{
    class current_thread_schedulable;
    class worker_strategy;

    using trampoline_schedulable = schedulable_wrapper<worker_strategy>;

    class worker_strategy
    {
    public:
        explicit worker_strategy(const rpp::composite_subscription& subscription)
            : m_sub{ subscription } {}

        void defer_at(time_point time_point, constraint::schedulable_fn auto&& fn) const
        {
            if (!m_sub.is_subscribed())
                return;

            auto&      queue              = get_schedulable_queue();
            const bool someone_owns_queue = queue.has_value();

            const auto drain_on_exit =  utils::finally_action(!someone_owns_queue ? &drain_queue : +[]{});

            if (!someone_owns_queue)
            {
                queue = std::priority_queue<current_thread_schedulable>{};

                if (!details::immediate_scheduling_while_condition(time_point, fn, m_sub, []() { return get_schedulable_queue()->empty(); }))
                    return;

                // update time to make it more accurate due to we are going to push it to queue
                time_point = std::max(now(), time_point);
            }

            defer_at(time_point, trampoline_schedulable{ *this, time_point, std::forward<decltype(fn)>(fn) });
        }

        void defer_at(time_point time_point, trampoline_schedulable&& fn) const
        {
            if (!m_sub.is_subscribed())
                return;

            get_schedulable_queue()->emplace(time_point, std::move(fn), m_sub);
        }

        static time_point now() { return clock_type::now(); }
    private:
        rpp::composite_subscription m_sub;
    };

    static void drain_queue()
    {
        auto& queue = get_schedulable_queue();
        auto  reset_at_final = utils::finally_action{ [] { get_schedulable_queue().reset(); } };
        std::optional<trampoline_schedulable> function{};
        time_point prev_time_point{};

        while (!queue->empty())
        {
            const auto& top = queue->top();

            wait_and_extract_executable_if_subscribed(top, function, prev_time_point);

            // firstly we need to pop schedulable from queue due to execution of function can add new schedulable
            queue->pop();

            if (function)
                (*function)();

            function.reset();
        }
    }

    static void wait_and_extract_executable_if_subscribed(const current_thread_schedulable& schedulable, std::optional<trampoline_schedulable>& out, time_point& prev_time_point)
    {
        if (!schedulable.is_subscribed())
            return;

        const auto requested_time = schedulable.get_time_point();

        // wait only if needed!
        if (prev_time_point < requested_time)
        {
            std::this_thread::sleep_until(requested_time);
            prev_time_point = requested_time;

            if (!schedulable.is_subscribed())
                return;
        }

        out.emplace(std::move(schedulable.extract_function()));
    }

    class current_thread_schedulable : public details::schedulable<trampoline_schedulable>
    {
    public:
        current_thread_schedulable(time_point                  time_point,
                                   std::invocable auto&&       fn,
                                   rpp::composite_subscription subscription)
            : schedulable(time_point, get_thread_local_id(), std::forward<decltype(fn)>(fn))
            , m_subscription{std::move(subscription)} {}

        bool is_subscribed() const { return m_subscription.is_subscribed(); }

    private:
        static size_t get_thread_local_id()
        {
            static thread_local size_t s_id;
            return s_id++;
        }

    private:
        rpp::composite_subscription m_subscription{};
    };

    /**
     * \brief Returns optional thread_local schedulable queue. If optional has value -> someone just owns thread.
     */
    static std::optional<std::priority_queue<current_thread_schedulable>>& get_schedulable_queue()
    {
        static thread_local std::optional<std::priority_queue<current_thread_schedulable>> s_queue{};
        return s_queue;
    }

public:
    static bool is_queue_owned() { return get_schedulable_queue().has_value(); }

    static auto create_worker(const rpp::composite_subscription& sub = composite_subscription{})
    {
        return worker<worker_strategy>{sub};
    }
};
} // namespace rpp::schedulers
