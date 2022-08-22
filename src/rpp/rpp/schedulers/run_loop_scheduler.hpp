//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/schedulers/fwd.hpp>                       // own forwarding
#include <rpp/schedulers/details/worker.hpp>            // worker
#include <rpp/subscriptions/composite_subscription.hpp> // lifetime
#include <rpp/schedulers/details/queue_worker_state.hpp>// state


namespace rpp::schedulers
{
/**
 * \brief scheduler which schedules execution via queueing tasks, but execution of tasks should be manually dispatched
 * \warning you need manually dispatch events for this scheduler in some thread.
 * 
 * \par Example
 * \snippet run_loop.cpp run_loop
 * 
 * \ingroup schedulers
 */
class run_loop final : public details::scheduler_tag
{
private:
    class worker_strategy;
    using run_loop_schedulable = schedulable_wrapper<worker_strategy>;

    class worker_strategy
    {
    public:
        worker_strategy(const std::shared_ptr<details::queue_worker_state<run_loop_schedulable>>& queue,
                        const composite_subscription&                                             sub)
            : m_queue{queue}
            , m_sub{sub} { }

        void defer_at(time_point time_point, constraint::schedulable_fn auto&& fn) const
        {
            defer_at(time_point, run_loop_schedulable{*this, time_point, std::forward<decltype(fn)>(fn)});
        }

        void defer_at(time_point time_point, run_loop_schedulable&& fn) const
        {
            if (m_sub.is_subscribed())
                m_queue->emplace(time_point, std::move(fn));
        }

        static time_point now() { return clock_type::now(); }

    private:
        std::shared_ptr<details::queue_worker_state<run_loop_schedulable>> m_queue{};
        composite_subscription                                             m_sub{};
    };

    class state
    {
    public:
        state(const composite_subscription& sub = composite_subscription{})
            : m_sub{sub} { }

        state(const state&)     = delete;
        state(state&&) noexcept = delete;

        ~state()
        {
            m_source.request_stop();
            m_sub.unsubscribe();
        }

        details::queue_worker_state<run_loop_schedulable>& get_queue() { return m_queue; }
        std::stop_token                                    get_token() const { return m_source.get_token(); }

        const composite_subscription& get_subscription() const { return m_sub; }
    private:
        rpp::composite_subscription                       m_sub;
        details::queue_worker_state<run_loop_schedulable> m_queue{};
        std::stop_source                                  m_source{};
    };

public:


    run_loop(const composite_subscription& sub = composite_subscription{})
        : m_state(std::make_shared<state>(sub)) {}

    auto create_worker(const composite_subscription& sub = composite_subscription{}) const
    {
        auto res = m_state->get_subscription().add(sub);
        sub.add([weak = std::weak_ptr{m_state}, res]
        {
            if (const auto sh = weak.lock())
                sh->get_subscription().remove(res);
        });
        return worker<worker_strategy>{std::shared_ptr<details::queue_worker_state<run_loop_schedulable>>{m_state, &m_state->get_queue()}, sub};
    }

    bool is_empty() const
    {
        return m_state->get_queue().is_empty();
    }

    bool is_any_ready_schedulable() const
    {
        return m_state->get_queue().is_any_ready_schedulable();
    }

    void dispatch_if_ready() const
    {
        std::optional<run_loop_schedulable> fn{};
        if (m_state->get_queue().pop_if_ready(fn))
            (*fn)();
    }

    void dispatch() const
    {
        std::optional<run_loop_schedulable> fn{};
        if (m_state->get_queue().pop_with_wait(fn, m_state->get_token()))
            (*fn)();
    }

private:
    const std::shared_ptr<state> m_state{};
};
} // namespace rpp::schedulers
