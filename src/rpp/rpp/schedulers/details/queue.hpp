//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/schedulers/fwd.hpp>
#include <rpp/disposables/base_disposable.hpp>
#include <rpp/utils/utils.hpp>

#include <condition_variable>
#include <queue>
#include <memory>
#include <tuple>
#include <utility>

namespace rpp::schedulers::details
{
class schedulable_wrapper
{
public:
    template<rpp::constraint::observer TObs, typename...Args, constraint::schedulable_fn<TObs, Args...> Fn>
    schedulable_wrapper(Fn&& fn, TObs&& observer, Args&&... args)
        : m_data{std::make_shared<proxy<std::decay_t<Fn>, std::decay_t<TObs>, std::decay_t<Args>...>>(std::forward<Fn>(fn), std::forward<TObs>(observer), std::forward<Args>(args)...)}
        , m_vtable(&vtable::create<proxy<std::decay_t<Fn>, std::decay_t<TObs>, std::decay_t<Args>...>>())
    {
    }

    optional_duration operator()() const { return m_vtable->invoke(m_data.get()); }
    bool              is_disposed() const { return m_vtable->is_disposed(m_data.get()); }

private:
    template<typename Fn, typename TObs, typename... Args>
    struct proxy
    {
        Fn                        fn{};
        std::tuple<TObs, Args...> args{};

        optional_duration invoke() { return std::apply(fn, args); }
        bool              is_disposed() const { return std::get<0>(args).is_disposed(); }
    };

    struct vtable
    {
        optional_duration (*invoke)(void*){};
        bool (*is_disposed)(const void*){};

        template<typename Proxy>
        static const vtable* create() noexcept
        {
            static vtable s_res{
                .invoke = +[](void* proxy) { return static_cast<Proxy*>(proxy)->invoke(); },
                .is_disposed = +[](const void* proxy) { return static_cast<const Proxy*>(proxy)->is_disposed(); }
            };
            return &s_res;
        }
    };
    std::shared_ptr<void> m_data{};
    vtable*               m_vtable{};
};

class schedulable
{
public:
    schedulable(time_point time_point, size_t id, schedulable_wrapper&& fn)
        : m_function{std::move(fn)}
        , m_time_point{time_point}
        , m_id{id}
    {}

    schedulable(const schedulable& other)                = default;
    schedulable(schedulable&& other) noexcept            = default;
    schedulable& operator=(const schedulable& other)     = default;
    schedulable& operator=(schedulable&& other) noexcept = default;

    bool operator<(const schedulable& other) const
    {
        return std::tie(m_time_point, m_id) >= std::tie(other.m_time_point, other.m_id);
    }

    time_point            get_time_point() const { return m_time_point; }
    const schedulable_wrapper& get_function() const { return m_function; }

private:
    schedulable_wrapper m_function;
    time_point          m_time_point;
    size_t              m_id;
};

template<typename Mutex>
class queue final : public base_disposable
{
public:
    queue() = default;
    ~queue() noexcept { dispose(); }

    template<rpp::constraint::observer TObs, typename...Args, constraint::schedulable_fn<TObs, Args...> Fn>
    void emplace(const duration duration, Fn&& fn, TObs&& obs, Args&&...args)
    {
        emplace(duration, schedulable_wrapper{std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...});
    }

    void emplace(const duration duration, schedulable_wrapper&& wrapper)
    {
        {
            std::lock_guard lock{m_mutex};
            if (!is_disposed())
                m_queue.emplace(clock_type::now()+duration, m_current_id++,std::move(wrapper));
        }
        m_cv.notify_one();
    }

    bool is_empty() const
    {
        std::lock_guard lock{ m_mutex };
        return m_queue.empty();
    }

    bool dispatch_if_ready()
    {
        std::lock_guard lock{ m_mutex };
        if (!is_any_ready_schedulable_unsafe())
            return false;

        auto fn = m_queue.top().get_function();
        m_queue.pop();
        lock.unlock();

        dispatch_impl(std::move(fn));
        return true;
    }

    void dispatch()
    {
        while (!is_disposed())
        {
            std::unique_lock lock{m_mutex};
            m_cv.wait(lock, [&] { return !m_queue.empty() || is_disposed(); });

            if (m_queue.empty() || !m_cv.wait_until(lock, m_queue.top().get_time_point(), [&] { return is_any_ready_schedulable_unsafe() || is_disposed(); }))
                continue;

            if (is_disposed())
                return;

            auto fn = m_queue.top().get_function();
            m_queue.pop();
            lock.unlock();

            dispatch_impl(std::move(fn));
            return;
        }
    }

private:
    void dispatch_impl(schedulable_wrapper&& fn)
    {
        if (fn.is_disposed())
            return;

        if (const auto duration = fn())
        {
            emplace(duration.value(), std::move(fn));
        }
    }

    bool is_any_ready_schedulable_unsafe()
    {
        while (!m_queue.empty())
        {
            if (m_queue.top().get_function().is_disposed())
                m_queue.pop();
            else
                return m_queue.top().get_time_point() <= clock_type::now();
        }
        return false;
    }

    void dispose_impl() override
    {
        {
            std::lock_guard lock{m_mutex};
            m_queue = std::priority_queue<schedulable>{};
        }
        m_cv.notify_one();
    }

private:
    std::priority_queue<schedulable> m_queue{};
    Mutex                            m_mutex{};
    std::condition_variable          m_cv{};
    size_t                           m_current_id{};
};

using mutex_queue = queue<std::mutex>;
using none_lock_queue = queue<rpp::utils::none_lock>;
}