//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/schedulers/fwd.hpp>

#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/disposables/details/base_disposable.hpp>
#include <rpp/schedulers/current_thread.hpp>
#include <rpp/schedulers/details/worker.hpp>
#include <rpp/schedulers/details/queue.hpp>
#include <rpp/utils/functors.hpp>

namespace rpp::schedulers
{
/**
 * @brief scheduler which schedules execution via queueing tasks, but execution of tasks should be manually dispatched
 * @warning you need manually dispatch events for this scheduler in some thread.
 *
 * @ingroup schedulers
 */
class run_loop final
{
    class worker_strategy;
    
    class state_t final : public rpp::details::base_disposable
    {
    public:
        ~state_t() noexcept override { dispose(); }

        template<typename ...Args>
        void emplace_and_notify(time_point timepoint, Args&& ...args)
        {
            if (is_disposed())
                return;

            {
                std::lock_guard lock{m_mutex};
                m_queue.emplace(timepoint, std::forward<Args>(args)...);
            }
            m_cv.notify_one();
        }

        std::shared_ptr<details::schedulable_base> pop(bool wait)
        {
            while(!is_disposed())
            {
                std::unique_lock lock{m_mutex};
                m_cv.wait(lock, [&] { return !wait || is_disposed() || !m_queue.is_empty(); });

                if (is_disposed())
                    break;
            
                const auto now = worker_strategy::now();
                if (is_any_ready_schedulable_unsafe(now))
                    return m_queue.pop();
                
                if (!wait)
                    break;
                
                m_cv.wait_for(lock, m_queue.top()->get_timepoint() - now, [&](){ return is_disposed() || !m_queue.is_empty() || m_queue.top()->get_timepoint() <= worker_strategy::now(); });
            }
            return {};
        }

        bool is_any_ready_schedulable() 
        {
            std::lock_guard lock{m_mutex};
            return is_any_ready_schedulable_unsafe();
        }

        bool is_empty() 
        {
            std::lock_guard lock{m_mutex};
            return m_queue.is_empty();
        }

    private:
        bool is_any_ready_schedulable_unsafe(time_point now = worker_strategy::now()) const
        {
            return !m_queue.is_empty() && (m_queue.top()->is_disposed() || m_queue.top()->get_timepoint() <= now);
        }

        void dispose_impl() noexcept override 
        {
            {
                std::lock_guard lock{m_mutex};
                m_queue = details::schedulables_queue<worker_strategy>{};
            }
            m_cv.notify_one();
        }

    private:
        std::mutex                  m_mutex{};
        details::schedulables_queue<worker_strategy> m_queue{};

        std::condition_variable     m_cv{};
    };

    class worker_strategy
    {
    public:
        worker_strategy(const std::weak_ptr<state_t>& state) 
            : m_state{state}
        {}

        template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, constraint::schedulable_fn<Handler, Args...> Fn>
        void defer_for(duration duration, Fn&& fn, Handler&& handler, Args&&... args) const
        {
            if (const auto shared = m_state.lock())
                shared->emplace_and_notify(now()+duration, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
        }

        static constexpr rpp::schedulers::details::none_disposable get_disposable() { return {}; }

        static rpp::schedulers::time_point now() { return details::now(); }
    private:
        std::weak_ptr<state_t> m_state;
    };

public:
    bool is_empty() const
    {
        return m_state->is_empty();
    }

    bool is_any_ready_schedulable() const
    {
        return m_state->is_any_ready_schedulable();
    }

    void dispatch_if_ready() const
    {
        dispatch_impl(false);
    }

    void dispatch() const
    {
        dispatch_impl(true);
    }

    rpp::schedulers::worker<worker_strategy> create_worker() const
    {
        return rpp::schedulers::worker<worker_strategy>{m_state};
    }

private:
    void dispatch_impl(bool wait) const
    {
        if (auto top = m_state->pop(wait)) {
            if (top->is_disposed())
                return;

            if (const auto timepoint = (*top)())
                m_state->emplace_and_notify(timepoint.value(), std::move(top));
        }
    }

private:
    std::shared_ptr<state_t> m_state = std::make_shared<state_t>();
};
}
