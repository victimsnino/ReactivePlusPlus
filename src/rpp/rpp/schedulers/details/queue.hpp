//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/defs.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/schedulers/fwd.hpp>
#include <rpp/disposables/base_disposable.hpp>
#include <rpp/utils/utils.hpp>

#include <condition_variable>
#include <exception>
#include <optional>
#include <queue>
#include <memory>
#include <tuple>
#include <utility>

namespace rpp::schedulers::details
{
class schedulable_base
{
public:
    explicit schedulable_base(const time_point& time_point) : m_time_point{time_point} {}

    virtual ~schedulable_base() noexcept = default;

    virtual optional_duration operator()()        = 0;
    virtual bool              is_disposed() const = 0;

    time_point get_timepoint() const { return m_time_point; }
    void       set_timepoint(const time_point& timepoint) { m_time_point = timepoint; }

    const std::shared_ptr<schedulable_base>& get_next() const { return m_next; }
    void set_next(std::shared_ptr<schedulable_base>&& next) { m_next = std::move(next); }

    void update_next(std::shared_ptr<schedulable_base>&& next)
    {
        if (next)
            next->set_next(std::move(m_next));
        m_next = std::move(next);
    }

private:
    std::shared_ptr<schedulable_base> m_next{};
    time_point                        m_time_point;
};

template<rpp::constraint::decayed_type Fn, rpp::constraint::observer TObs, rpp::constraint::decayed_type... Args>
    requires constraint::schedulable_fn<Fn, TObs, Args...>
class specific_schedulable final : public schedulable_base
{
public:
    template<rpp::constraint::decayed_same_as<Fn> TFn, rpp::constraint::decayed_same_as<TObs> TTObs, typename... TArgs>
    explicit specific_schedulable(const time_point& time_point, TFn&& in_fn, TTObs&& in_obs, TArgs&&... in_args)
        : schedulable_base{time_point}
        , m_args(std::forward<TTObs>(in_obs), std::forward<TArgs>(in_args)...)
        , m_fn{std::forward<TFn>(in_fn)}
    {
    }

    optional_duration operator()() override
    {
        try
        {
            return std::apply(m_fn, m_args);
        }
        catch(...)
        {
            std::get<0>(m_args).on_error(std::current_exception());
            return std::nullopt;
        }
    }
    bool is_disposed() const override { return std::get<0>(m_args).is_disposed(); }

private:
    std::tuple<TObs, Args...> m_args;
    RPP_NO_UNIQUE_ADDRESS Fn  m_fn;
};

class schedulables_queue
{
public:
    template<rpp::constraint::observer TObs, typename... Args, constraint::schedulable_fn<TObs, Args...> Fn>
    void emplace(const time_point& timepoint, Fn&& fn, TObs&& obs, Args&&... args)
    {
        using schedulable_type = specific_schedulable<std::decay_t<Fn>, std::decay_t<TObs>, std::decay_t<Args>...>;

        emplace_impl(std::make_shared<schedulable_type>(timepoint, std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...));
    }

    void emplace(const time_point& timepoint, std::shared_ptr<schedulable_base>&& schedulable)
    {
        if (!schedulable)
            return;

        schedulable->set_timepoint(timepoint);
        emplace_impl(std::move(schedulable));
    }

    bool is_empty() const { return !m_head; }

    std::shared_ptr<schedulable_base> pop()
    {
        return std::exchange(m_head, m_head->get_next());
    }

    const std::shared_ptr<schedulable_base>& top() const
    {
        return m_head;
    }

private:
    void emplace_impl(std::shared_ptr<schedulable_base>&& schedulable)
    {
        if (!m_head || schedulable->get_timepoint() < m_head->get_timepoint())
        {
            schedulable->set_next(std::move(m_head));
            m_head = std::move(schedulable);
            return;
        }

        schedulable_base* current = m_head.get();
        while (const auto& next = current->get_next())
        {
            if (schedulable->get_timepoint() < next->get_timepoint())
                break;
            current = next.get();
        }

        current->update_next(std::move(schedulable));
    }
private:
    std::shared_ptr<schedulable_base> m_head{};
};

// class queue final : public base_disposable
// {
// public:
//     queue() = default;
//     ~queue() override noexcept  { dispose(); }

//     template<rpp::constraint::observer TObs, typename...Args, constraint::schedulable_fn<TObs, Args...> Fn>
//     void emplace(const duration duration, Fn&& fn, TObs&& obs, Args&&...args)
//     {
//         emplace(duration, schedulable_wrapper{std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...});
//     }

//     void emplace(const duration duration, schedulable_wrapper&& wrapper)
//     {
//         {
//             std::lock_guard lock{m_mutex};
//             if (!is_disposed())
//                 m_queue.emplace(clock_type::now()+duration, m_current_id++,std::move(wrapper));
//         }
//         m_cv.notify_one();
//     }

//     bool is_empty() const
//     {
//         std::lock_guard lock{ m_mutex };
//         return m_queue.empty();
//     }

//     bool dispatch_if_ready()
//     {
//         std::unique_lock lock{ m_mutex };
//         return dispatch_if_ready_impl(lock);
//     }

//     void dispatch()
//     {
//         while (!is_disposed())
//         {
//             std::unique_lock lock{m_mutex};
//             m_cv.wait(lock, [&] { return !m_queue.empty() || is_disposed(); });

//             if (m_queue.empty())
//                 continue;

//             if (m_queue.top().get_function().is_disposed())
//             {
//                 m_queue.pop();
//                 return;
//             }

//             if (m_cv.wait_until(lock,
//                                 m_queue.top().get_time_point(),
//                                 [&] { return dispatch_if_ready_impl(lock) || is_disposed(); }))
//                 return;
//         }
//     }

// private:
//     bool dispatch_if_ready_impl(std::unique_lock<std::mutex>& lock)
//     {
//         if (!is_any_ready_schedulable_unsafe())
//             return false;

//         auto fn = m_queue.top().get_function();
//         m_queue.pop();
//         lock.unlock();

//         if (!fn.is_disposed())
//         {
//             if (const auto duration = fn())
//             {
//                 emplace(duration.value(), std::move(fn));
//             }
//         }
//         return true;
//     }

//     bool is_any_ready_schedulable_unsafe()
//     {
//         return !m_queue.empty() && m_queue.top().get_time_point() <= clock_type::now();
//     }

//     void dispose_impl() override
//     {
//         {
//             std::lock_guard lock{m_mutex};
//             m_queue = std::priority_queue<schedulable>{};
//         }
//         m_cv.notify_one();
//     }

// private:
//     std::priority_queue<schedulable> m_queue{};
//     mutable std::mutex               m_mutex{};
//     std::condition_variable          m_cv{};
//     size_t                           m_current_id{};
// };
}