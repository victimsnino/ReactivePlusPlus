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

#include <rpp/schedulers/fwd.h>
#include <rpp/schedulers/constraints.h>

namespace rpp::schedulers
{
template<typename T>
concept worker_strategy = std::copyable<T> && requires(const T t)
{
    t.defer_at(time_point{}, []() {});
};

template<worker_strategy Strategy>
class worker
{
public:
    template<typename ...Args>
    worker(Args&& ...args) : m_strategy{std::forward<Args>(args)...} {}

    void schedule(constraint::schedulable_fn auto&& fn) const
    {
        schedule(std::chrono::high_resolution_clock::now(), std::forward<decltype(fn)>(fn));
    }

    void schedule(time_point time_point, constraint::schedulable_fn auto&& fn) const
    {
        m_strategy.defer_at(time_point, scheduler_wrapper{m_strategy, time_point, std::forward<decltype(fn)>(fn)});
    }

private:
    template<constraint::schedulable_fn Fn>
    struct scheduler_wrapper
    {
        scheduler_wrapper(const Strategy& strategy, time_point time_point, const Fn& fn)
            : m_strategy{strategy}
            , m_time_point{time_point}
            , m_fn{fn} {}

        scheduler_wrapper(const Strategy& strategy, time_point time_point, Fn&& fn)
            : m_strategy{strategy}
            , m_time_point{time_point}
            , m_fn{std::move(fn)} {}

        scheduler_wrapper(const scheduler_wrapper&) = default;
        scheduler_wrapper(scheduler_wrapper&&)      = default;

        void operator()()
        {
            if (auto duration = m_fn())
            {
                m_time_point += duration.value();
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