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

#include <rpp/subscription.h>
#include <rpp/schedulers/constraints.h>
#include <rpp/schedulers/fwd.h>

#include <chrono>
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
            : m_time_point{ time_point }
            , m_id{ id }
            , m_function{ std::forward<decltype(fn)>(fn) } {}

        schedulable(const schedulable& other)                = default;
        schedulable(schedulable&& other) noexcept            = default;
        schedulable& operator=(const schedulable& other)     = default;
        schedulable& operator=(schedulable&& other) noexcept = default;

        bool operator<(const schedulable& other) const
        {
            if (m_time_point != other.m_time_point)
                return m_time_point > other.m_time_point;
            return m_id > other.m_id;
        }

        time_point                           GetTimePoint() const { return m_time_point; }
        std::function<optional_duration()>&& ExtractFunction() const { return std::move(m_function); }

    private:
        time_point                         m_time_point;
        size_t                             m_id;
        mutable std::function<optional_duration()> m_function;
    };

    class new_thread final : public details::scheduler_tag
    {
    public:
        class worker
        {
        public:
            worker(rpp::subscription&& sub)
                : m_sub{std::move(sub)}
                , m_thread{std::bind_front(&worker::data_thread, this)} {}

            ~worker()
            {
                if (m_thread.joinable())
                {
                    m_thread.request_stop();
                    m_cv.notify_one();
                    m_thread.join();
                }
            }

            void schedule(constraint::schedulable_fn auto&& fn)
            {
                schedule(std::chrono::high_resolution_clock::now(), std::forward<decltype(fn)>(fn));
            }

            void schedule(time_point time_point, constraint::schedulable_fn auto&& fn)
            {
                if (!m_sub.is_subscribed())
                    return;

                {
                    std::lock_guard lock{ m_mutex };
                    m_queue.emplace(time_point, ++m_current_id, std::forward<decltype(fn)>(fn));
                }
                m_cv.notify_one();
            }
        private:
            void data_thread(std::stop_token token)
            {
                while (!token.stop_requested())
                {
                    std::unique_lock lock{m_mutex};
                    if (m_queue.empty())
                    {
                        m_cv.wait(lock);
                    }
                    else
                    {
                        m_cv.wait_until(lock, m_queue.top().GetTimePoint());
                    }

                    if (token.stop_requested())
                        return;

                    if (m_queue.empty() || m_queue.top().GetTimePoint() > clock_type::now())
                        continue;

                    auto fn         = std::move(m_queue.top().ExtractFunction());
                    auto time_point = m_queue.top().GetTimePoint();
                    m_queue.pop();

                    lock.unlock();

                    auto duration = fn();
                    if (!duration.has_value())
                        continue;
                    time_point += duration.value();
                    schedule(time_point, std::move(fn));
                }
            }

        private:
            rpp::subscription m_sub;
            
            std::mutex                       m_mutex{};
            std::condition_variable          m_cv{};
            std::priority_queue<schedulable> m_queue{};
            size_t                           m_current_id{};
            std::jthread                     m_thread{};
        };

        static auto create_worker(rpp::subscription sub = {})
        {
            return std::make_shared<worker>(std::move(sub));
        }
    };
} // namespace rpp::schedulers
