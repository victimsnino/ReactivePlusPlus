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

#include <rpp/schedulers/fwd.h>
#include <rpp/schedulers/worker.h>
#include <rpp/subscriptions/composite_subscription.h>
#include <rpp/subscriptions/subscription_guard.h>

#include <concepts>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace rpp::schedulers
{
class schedulable
{
public:
    schedulable(time_point time_point, size_t id, std::invocable auto&& fn)
        : m_time_point{time_point}
        , m_id{id}
        , m_function{std::forward<decltype(fn)>(fn)} {}

    schedulable(const schedulable& other)                = default;
    schedulable(schedulable&& other) noexcept            = default;
    schedulable& operator=(const schedulable& other)     = default;
    schedulable& operator=(schedulable&& other) noexcept = default;

    bool operator<(const schedulable& other) const
    {
        return std::tie(m_time_point, m_id) > std::tie(other.m_time_point, other.m_id);
    }

    time_point              GetTimePoint() const { return m_time_point; }
    std::function<void()>&& ExtractFunction() const { return std::move(m_function); }

private:
    time_point                    m_time_point;
    size_t                        m_id;
    mutable std::function<void()> m_function;
};

/**
 * \brief scheduler which schedule execution of via queueing tasks to another thread with priority to time_point and order
 * \details Creates new thread for each "create_worker" call. Any scheduled task will be queued to created thread for execution with respect to time_point and number of task
 */
class new_thread final : public details::scheduler_tag
{
public:
    class worker_strategy
    {
    public:
        worker_strategy(const rpp::composite_subscription& sub)
            : m_state{std::make_shared<state>()}
        {
            m_state->thread = std::jthread{[state = m_state](const std::stop_token& token)
            {
                state->data_thread(token);
            }};
            m_state->sub.reset(sub.add([state = m_state]
            {
                state->thread.request_stop();

                if (state->thread.get_id() != std::this_thread::get_id())
                    state->thread.join();
                else
                    state->thread.detach();
            }));
        }

        void defer_at(time_point time_point, std::invocable auto&& fn) const
        {
            m_state->defer_at(time_point, std::forward<decltype(fn)>(fn));
        }

    private:
        struct state
        {
            void defer_at(time_point time_point, std::invocable auto&& fn)
            {
                if (!sub->is_subscribed())
                    return;

                {
                    std::lock_guard lock{mutex};
                    queue.emplace(time_point, ++current_id, std::forward<decltype(fn)>(fn));
                }
                cv.notify_one();
            }

            void data_thread(const std::stop_token& token)
            {
                std::function<void()> fn{};
                while (!token.stop_requested())
                {
                    std::unique_lock lock{ mutex };

                    if (!cv.wait(lock, token, [&] { return !queue.empty(); }))
                        continue;

                    if (!cv.wait_until(lock, token, queue.top().GetTimePoint(), [&] { return !queue.empty() && queue.top().GetTimePoint() <= clock_type::now(); }))
                        continue;

                    fn = std::move(queue.top().ExtractFunction());
                    queue.pop();

                    lock.unlock();

                    fn();
                    fn = {};
                }
            }

            std::mutex                       mutex{};
            std::condition_variable_any      cv{};
            std::priority_queue<schedulable> queue{};
            size_t                           current_id{};
            std::jthread                     thread{};
            rpp::subscription_guard          sub = rpp::subscription_base::empty();
        };

        std::shared_ptr<state> m_state{};
    };

    static auto create_worker(const rpp::composite_subscription& sub = composite_subscription{})
    {
        return worker<worker_strategy>{sub};
    }
};
} // namespace rpp::schedulers
