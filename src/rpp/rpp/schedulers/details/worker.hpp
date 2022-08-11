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

#include <rpp/schedulers/constraints.hpp> // schedulable_fn
#include <rpp/schedulers/fwd.hpp>         // own forwarding
#include <rpp/utils/constraints.hpp>

#include <algorithm>
#include <functional>

namespace rpp::schedulers
{
template<typename T>
concept worker_strategy = std::copyable<T> && requires(const T t)
{
    t.defer_at(time_point{}, std::declval<optional_duration(*)()>());
    { t.now() } -> std::same_as<time_point>;
};

template<typename Strategy>
class schedulable_wrapper
{
public:
    template<constraint::schedulable_fn Fn>
    schedulable_wrapper(const Strategy& strategy, time_point time_point, Fn&& fn)
        : m_strategy{strategy}
        , m_time_point{time_point}
        , m_fn{std::forward<Fn>(fn)} {}

    void operator()()
    {
        if (auto duration = m_fn())
        {
            m_time_point = std::max(m_strategy.now(), m_time_point + duration.value());

            m_strategy.defer_at(m_time_point, std::move(*this));
        }
    }

private:
    Strategy                           m_strategy;
    time_point                         m_time_point;
    std::function<optional_duration()> m_fn{};
};

template<worker_strategy Strategy>
class worker final : public details::worker_tag
{
public:
    template<typename ...Args>
        requires (!rpp::constraint::variadic_is_same_type<worker<Strategy>, Args...>)
    worker(Args&& ...args) : m_strategy{std::forward<Args>(args)...} {}

    worker(const worker&) = default;
    worker(worker&&) noexcept= default;

    void schedule(constraint::schedulable_fn auto&& fn) const
    {
        schedule(m_strategy.now(), std::forward<decltype(fn)>(fn));
    }

    void schedule(duration delay, constraint::schedulable_fn auto&& fn) const
    {
        schedule(m_strategy.now() + delay, std::forward<decltype(fn)>(fn));
    }

    void schedule(time_point time_point, constraint::schedulable_fn auto&& fn) const
    {
        m_strategy.defer_at(time_point, std::forward<decltype(fn)>(fn));
    }

private:
    Strategy m_strategy;
};
} // namespace rpp::schedulers