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

namespace rpp::schedulers
{
template<typename T>
concept worker_strategy = std::copyable<T> && requires(const T t)
{
    t.defer_at(time_point{}, std::declval<void(*)()>());
    { t.now() } -> std::same_as<time_point>;
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
        m_strategy.defer_at(time_point, schedulable_wrapper<std::decay_t<decltype(fn)>>{m_strategy, time_point, std::forward<decltype(fn)>(fn)});
    }

private:
    template<constraint::schedulable_fn Fn>
    struct schedulable_wrapper
    {
        schedulable_wrapper(const Strategy& strategy, time_point time_point, const Fn& fn)
            : m_strategy{strategy}
            , m_time_point{time_point}
            , m_fn{fn} {}

        schedulable_wrapper(const Strategy& strategy, time_point time_point, Fn&& fn)
            : m_strategy{strategy}
            , m_time_point{time_point}
            , m_fn{std::move(fn)} {}

        schedulable_wrapper(const schedulable_wrapper&)                                               = default; // LCOV_EXCL_LINE
        schedulable_wrapper(schedulable_wrapper&&) noexcept(std::is_nothrow_move_constructible_v<Fn>) = default;

        void operator()()
        {
            if (auto duration = m_fn())
            {
                if (duration.value() != rpp::schedulers::duration::zero())
                    m_time_point += duration.value();
                else
                    m_time_point = m_strategy.now();

                auto time_to_schedule = m_time_point;
                m_strategy.defer_at(time_to_schedule, std::move(*this));
            }
        }

        Strategy   m_strategy;
        time_point m_time_point;
        Fn         m_fn{};
    };
private:
    Strategy m_strategy;
};
} // namespace rpp::schedulers