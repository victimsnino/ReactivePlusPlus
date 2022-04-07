// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "rpp/subscriptions/subscription_guard.h"

#include <rpp/subscriptions/composite_subscription.h>
#include <rpp/schedulers/constraints.h>
#include <rpp/schedulers/fwd.h>

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
    schedulable(time_point time_point, size_t id, constraint::schedulable_fn auto&& fn)
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

    time_point                           GetTimePoint() const { return m_time_point; }
    std::function<optional_duration()>&& ExtractFunction() const { return std::move(m_function); }

private:
    time_point                                 m_time_point;
    size_t                                     m_id;
    mutable std::function<optional_duration()> m_function;
};

/**
 * \brief scheduler which schedule execution of via queueing tasks to another thread with priority to time_point and order
 * \details Creates new thread for each "create_worker" call. Any scheduled task will be queued to created thread for execution with respect to time_point and number of task
 */
class new_thread final : public details::scheduler_tag
{
public:
    class worker
    {
    public:
        worker(const rpp::composite_subscription& sub)
            : m_state{std::make_shared<state>()}
        {
            m_state->thread = std::jthread{[state = m_state](const std::stop_token& token)
            {
                state->data_thread(token);
            }};
            m_state->sub = sub.add([state = m_state]
            {
                state->thread.request_stop();

                if (state->thread.get_id() != std::this_thread::get_id())
                    state->thread.join();
                else
                    state->thread.detach();
            });
        }

        void schedule(constraint::schedulable_fn auto&& fn)
        {
            schedule(std::chrono::high_resolution_clock::now(), std::forward<decltype(fn)>(fn));
        }

        void schedule(time_point time_point, constraint::schedulable_fn auto&& fn)
        {
            m_state->schedule(time_point, std::forward<decltype(fn)>(fn));
        }

    private:
        struct state
        {
            void schedule(time_point time_point, constraint::schedulable_fn auto&& fn)
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
                std::function<optional_duration()> fn{};
                time_point                         time_point{};
                while (!token.stop_requested())
                {
                    {
                        std::unique_lock lock{mutex};

                        if (!cv.wait(lock, token, [&] { return !queue.empty(); }))
                            continue;

                        if (!cv.wait_until(lock, token, queue.top().GetTimePoint(), [&] { return !queue.empty() && queue.top().GetTimePoint() <= clock_type::now(); }))
                            continue;

                        fn         = std::move(queue.top().ExtractFunction());
                        time_point = queue.top().GetTimePoint();
                        queue.pop();
                    }

                    if (auto duration = fn())
                    {
                        time_point += duration.value();
                        schedule(time_point, std::move(fn));
                    }
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
        return worker{sub};
    }
};
} // namespace rpp::schedulers
