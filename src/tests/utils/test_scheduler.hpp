//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/schedulers.hpp>

static rpp::schedulers::time_point s_current_time{std::chrono::seconds{10}};

class test_scheduler final : public rpp::schedulers::details::scheduler_tag
{
public:
    struct state
    {
        state() = default;

        void schedule(rpp::schedulers::time_point                        time_point,
                      rpp::schedulers::constraint::schedulable_fn auto&& fn)
        {
            if (!sub.is_subscribed())
                return;

            schedulings.push_back(time_point);
            queue.emplace(time_point,
                          static_cast<size_t>(rpp::schedulers::clock_type::now().time_since_epoch().count()),
                          std::forward<decltype(fn)>(fn));
        }

        void drain()
        {
            while (!queue.empty() && sub.is_subscribed())
            {
                auto time_point = queue.top().get_time_point();
                if (time_point > s_current_time)
                    return;

                auto fn = queue.top().extract_function();
                queue.pop();

                executions.push_back(s_current_time);
                if (auto duration = fn())
                    schedule(std::max(s_current_time, time_point + duration.value()), std::move(fn));
            }
        }

        rpp::subscription_base                   sub{};
        std::vector<rpp::schedulers::time_point> schedulings{};
        std::vector<rpp::schedulers::time_point> executions{};
        std::priority_queue<rpp::schedulers::details::schedulable<std::function<rpp::schedulers::optional_duration()>>> queue{};
    };

    class worker_strategy
    {
    public:
        worker_strategy(std::weak_ptr<state> state)
            : m_state{state} { }

        void defer_at(rpp::schedulers::time_point                        time_point,
                      rpp::schedulers::constraint::schedulable_fn auto&& fn) const
        {
            if (auto locked = m_state.lock())
            {
                if (locked->sub.is_subscribed())
                {
                    locked->schedule(time_point, std::forward<decltype(fn)>(fn));
                    locked->drain();
                }
            }
        }

        static rpp::schedulers::time_point now() { return s_current_time; }

    private:
        std::weak_ptr<state> m_state;
    };

    test_scheduler() {}

    rpp::schedulers::worker<worker_strategy> create_worker(const rpp::subscription_base& sub = {}) const
    {
        m_state->sub = sub;
        return rpp::schedulers::worker<worker_strategy>{m_state};
    }

    const auto& get_schedulings() const { return m_state->schedulings; }
    const auto& get_executions() const { return m_state->executions; }

    void time_advance(rpp::schedulers::duration dur) const
    {
        s_current_time += dur;
        m_state->drain();
    }

private:
    std::shared_ptr<state> m_state = std::make_shared<state>();
};
