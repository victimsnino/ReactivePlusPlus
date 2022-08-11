//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/schedulers/fwd.hpp>  // own forwarding

#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

namespace rpp::schedulers::details
{
class queue_worker_state
{
public:
    queue_worker_state() noexcept = default;

    void emplace(time_point time_point, std::invocable auto&& fn)
    {
        emplace_safe(time_point, std::forward<decltype(fn)>(fn));
        m_cv.notify_one();
    }

    bool is_empty() const
    {
        std::lock_guard lock{ m_mutex };
        return m_queue.empty();
    }

    bool is_any_ready_schedulable() const
    {
        std::lock_guard lock{ m_mutex };
        return is_any_ready_schedulable_unsafe();
    }

    bool pop_if_ready(std::function<void()>& out)
    {
        std::lock_guard lock{ m_mutex };
        if (!is_any_ready_schedulable_unsafe())
            return false;

        out = std::move(m_queue.top().ExtractFunction());
        m_queue.pop();
        return true;
    }

    bool pop_with_wait(std::function<void()>& out, const std::stop_token& token)
    {
        while (!token.stop_requested())
        {
            std::unique_lock lock{m_mutex};

            if (!m_cv.wait(lock, token, [&] { return !m_queue.empty(); }))
                continue;

            if (!m_cv.wait_until(lock,
                                 token,
                                 m_queue.top().GetTimePoint(),
                                 std::bind_front(&queue_worker_state::is_any_ready_schedulable_unsafe, this)))
                continue;

            out = std::move(m_queue.top().ExtractFunction());
            m_queue.pop();
            return true;
        }
        return false;
    }

    void reset()
    {
        std::lock_guard lock{ m_mutex };
        m_queue = std::priority_queue<schedulable>{};
    }

private:
    void emplace_safe(time_point time_point, std::invocable auto&& fn)
    {
        std::lock_guard lock{ m_mutex };
        m_queue.emplace(time_point, ++m_current_id, std::forward<decltype(fn)>(fn));
    }

    bool is_any_ready_schedulable_unsafe() const
    {
        return !m_queue.empty() && m_queue.top().GetTimePoint() <= clock_type::now();
    }

private:
    class schedulable
    {
    public:
        schedulable(time_point time_point, size_t id, std::invocable auto&& fn)
            : m_time_point{ time_point }
            , m_id{ id }
            , m_function{ std::forward<decltype(fn)>(fn) } {}

        schedulable(const schedulable& other) = default;
        schedulable(schedulable&& other) noexcept = default;
        schedulable& operator=(const schedulable& other) = default;
        schedulable& operator=(schedulable&& other) noexcept = default;

        bool operator<(const schedulable& other) const
        {
            return std::tie(m_time_point, m_id) >= std::tie(other.m_time_point, other.m_id);
        }

        time_point              GetTimePoint() const { return m_time_point; }
        std::function<void()>&& ExtractFunction() const { return std::move(m_function); }

    private:
        time_point                    m_time_point;
        size_t                        m_id;
        mutable std::function<void()> m_function;
    };

    mutable std::mutex               m_mutex{};
    std::condition_variable_any      m_cv{};
    std::priority_queue<schedulable> m_queue{};
    size_t                           m_current_id{};
};
} // namespace rpp::schedulers::details
