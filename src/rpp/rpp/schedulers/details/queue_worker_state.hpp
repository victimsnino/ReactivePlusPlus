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
#include <rpp/schedulers/constraints.hpp>

#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

namespace rpp::schedulers::details
{
template<typename SchedulableFn>
class schedulable
{
public:
    schedulable(time_point time_point, size_t id, SchedulableFn&& fn)
        : m_time_point{time_point}
        , m_id{id}
        , m_function{std::move(fn)} {}

    schedulable(const schedulable& other)                = default;
    schedulable(schedulable&& other) noexcept            = default;
    schedulable& operator=(const schedulable& other)     = default;
    schedulable& operator=(schedulable&& other) noexcept = default;

    bool operator<(const schedulable& other) const
    {
        return std::tie(m_time_point, m_id) >= std::tie(other.m_time_point, other.m_id);
    }

    time_point      get_time_point() const { return m_time_point; }
    SchedulableFn&& extract_function() const { return std::move(m_function); }

private:
    time_point            m_time_point;
    size_t                m_id;
    mutable SchedulableFn m_function;
};

template<typename SchedulableFn>
class queue_worker_state
{
public:
    queue_worker_state() = default;
    queue_worker_state(const queue_worker_state&) = delete;
    queue_worker_state(queue_worker_state&&) = delete;

    void emplace(time_point time_point, constraint::inner_schedulable_fn auto&& fn)
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

    bool pop_if_ready(std::optional<SchedulableFn>& out)
    {
        std::lock_guard lock{ m_mutex };
        if (!is_any_ready_schedulable_unsafe())
            return false;

        out.emplace(std::move(m_queue.top().extract_function()));
        m_queue.pop();
        return true;
    }

    bool pop_with_wait(std::optional<SchedulableFn>& out)
    {
        while (m_subscription->is_subscribed())
        {
            std::unique_lock lock{m_mutex};

            m_cv.wait(lock, [&] { return !m_subscription->is_subscribed() || !m_queue.empty(); });

            if (!m_cv.wait_until(lock,
                                 m_queue.top().get_time_point(),
                                 [&] { return !m_subscription->is_subscribed() || is_any_ready_schedulable_unsafe(); }))
                continue;

            if (!m_subscription->is_subscribed())
                break;

            out.emplace(std::move(m_queue.top().extract_function()));
            m_queue.pop();
            return true;
        }
        return false;
    }

    void destroy()
    {
       m_subscription->unsubscribe();
    }

private:
    void emplace_safe(time_point time_point, constraint::inner_schedulable_fn auto&& fn)
    {
        std::lock_guard lock{m_mutex};
        if (m_subscription->is_subscribed())
            m_queue.emplace(time_point, ++m_current_id, std::forward<decltype(fn)>(fn));
    }

    bool is_any_ready_schedulable_unsafe() const
    {
        return !m_queue.empty() && m_queue.top().get_time_point() <= clock_type::now();
    }

private:
    mutable std::mutex                              m_mutex{};
    std::condition_variable_any                     m_cv{};
    std::priority_queue<schedulable<SchedulableFn>> m_queue{};
    size_t                                          m_current_id{};
    subscription_guard                              m_subscription = callback_subscription{[&]
    {
        {
            std::lock_guard lock{m_mutex};
            m_queue = std::priority_queue<schedulable<SchedulableFn>>{};
        }
        m_cv.notify_one();
    }};
};
} // namespace rpp::schedulers::details
