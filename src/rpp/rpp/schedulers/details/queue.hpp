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

#include <queue>
#include <memory>

namespace rpp::schedulers::details
{
class schedulable_wrapper
{
public:
    template<typename Fn, typename... Args>
    schedulable_wrapper(Fn&& fn, Args&&... args)
        : m_data{std::make_shared<proxy<std::decay_t<Fn>, std::decay_t<Args>...>>(std::forward<Fn>(fn), std::forward<Args>(args)...)},
          m_vtable(&vtable::create<std::decay_t<Fn>, std::decay_t<Args>...>())
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

        bool is_disposed() const
        {
            return std::get<0>(args).is_disposed();
        }

        optional_duration invoke()
        {
            return std::apply(fn, args);
        }
    };

    struct vtable
    {
        optional_duration (*invoke)(void*){};
        bool (*is_disposed)(const void*){};

        template<typename Proxy>
        static const vtable* create() noexcept
        {
            static vtable s_res{
                .invoke = +[](const void* proxy) { return static_cast<Proxy*>(proxy)->invoke(); },
                .is_disposed = +[](const void* proxy) { return static_cast<Proxy*>(proxy)->is_disposed(); }
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
    schedulable_wrapper&& extract() { return std::move(m_function); }

private:
    schedulable_wrapper m_function;
    time_point          m_time_point;
    size_t              m_id;
};

class queue
{
public:
    queue() = default;

    template<rpp::constraint::observer TObs, typename...Args>
    void emplace(const time_point time_point, constraint::schedulable_fn<TObs, Args...> auto&& fn, TObs&& obs, Args&&...args) const
    {
        m_queue.emplace(Args &&args...)
    }
private:
    std::priority_queue<schedulable> m_queue{};
    size_t                           m_current_id{};
};
}