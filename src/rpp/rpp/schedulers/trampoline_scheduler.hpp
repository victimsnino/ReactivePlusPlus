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

#include <concepts>
#include <chrono>
#include <functional>
#include <thread>


namespace rpp::schedulers
{

template <typename F>
struct scope_guard {
    explicit scope_guard(F&& f) : m_f(std::move(f)) {}
    ~scope_guard() noexcept { m_f(); }
    F m_f;
};

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
public:
    class worker_strategy
    {
    public:
        explicit worker_strategy(const rpp::composite_subscription& subscription)
            : m_sub{subscription} {};

        void defer_at(time_point time_point, std::invocable auto&& fn) const
        {
            if (!m_sub.is_subscribed())
                return;

            with_thread_local_schedulable_queue([&](auto& queue, bool& queue_in_use)
            {
                queue.emplace(time_point, std::forward<decltype(fn)>(fn));

                if (queue_in_use)
                    return;

                queue_in_use = true;
                scope_guard cleanup([&]() noexcept
                {
                    // If error occurs, we still want the thread-local state to be reset.
                    queue_in_use = false;
                    if (!m_sub.is_subscribed())
                        queue.reset();
                });

                std::stop_source stop;
                auto stop_token = stop.get_token();
                m_sub.add([stop = std::move(stop)]() mutable
                {
                    // The following blocking-pop exits when subscription is unsubscribed in other thread.
                    stop.request_stop();
                });

                while (!queue.is_empty() && m_sub.is_subscribed())
                {
                    std::function<void()> schedulable{};
                    if (queue.pop_with_wait(schedulable, stop_token))
                    {
                        schedulable();
                        schedulable = {};
                    }
                }
            });
        }

        static time_point now() { return clock_type::now(); }

    private:
        rpp::composite_subscription m_sub;
    };

    static auto create_worker(const rpp::composite_subscription& sub = composite_subscription{})
    {
        return worker<worker_strategy>{sub};
    }

private:
    /**
     * \brief evaluates given lambda with the thread local queue and ownership-flag.
     *
     * \param action a lambda that is given with the thread local queue and ownership-flag.
     */
    static void with_thread_local_schedulable_queue(std::function<void(rpp::schedulers::details::queue_worker_state&, bool&)>&& action)
    {
        static thread_local rpp::schedulers::details::queue_worker_state queue{};
        static thread_local bool queue_in_use{false};
        action(queue, queue_in_use);
    }
};
} // namespace rpp::schedulers
