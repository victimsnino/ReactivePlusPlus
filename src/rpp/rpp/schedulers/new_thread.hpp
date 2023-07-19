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

#include "rpp/disposables/fwd.hpp"
#include <rpp/schedulers/current_thread.hpp>
#include <rpp/disposables/details/base_disposable.hpp>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>


namespace rpp::schedulers
{
/**
 * @brief Scheduler which schedules invoking of schedulables to another thread via queueing tasks with priority to time_point and order
 * @warning Creates new thread for each "create_worker" call, but not for each schedule
 * @ingroup schedulers
 */
class new_thread
{
    class disposable final : public rpp::composite_disposable
    {
    public:
        disposable() {
            while (!m_state->queue_ptr.load(std::memory_order_relaxed)) {};
        }

        ~disposable() override
        {
            dispose();
        }

        template<rpp::constraint::observer TObs, typename... Args, constraint::schedulable_fn<TObs, Args...> Fn>
        void defer_at(time_point time_point, Fn&& fn, TObs&& obs, Args&&... args)
        {
            if (is_disposed())
                return;

            {
                std::lock_guard lock{m_state->queue_mutex};
                if (const auto queue = m_state->queue_ptr.load(std::memory_order_relaxed))
                    queue->emplace(time_point, std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...);
            }
            m_state->cv.notify_all();
        }

    private:
        void dispose_impl() override
        {
            std::call_once(m_state->once,
                           [&]
                           {
                               m_state->is_disposed.store(true, std::memory_order_relaxed);

                               if (m_thread.get_id() != std::this_thread::get_id())
                               {
                                   m_state->cv.notify_all();
                                   m_thread.join();
                               }
                               else
                                   m_thread.detach();
                           });
        }
        struct state_t
        {
            // recursive due to we need to keep lock during invoking of schedulable, but there is chance of recursive scheduling
            std::recursive_mutex                      queue_mutex{};
            std::atomic<details::schedulables_queue*> queue_ptr{};
            std::condition_variable_any               cv{};
            std::atomic_bool                          is_disposed{};
            std::once_flag                            once{};
        };

        static void data_thread(std::shared_ptr<state_t> state)
        {
            std::unique_lock lock{state->queue_mutex};
            auto& queue = current_thread::s_queue;
            state->queue_ptr.store(&queue.emplace(), std::memory_order_relaxed);

            while (!state->is_disposed.load(std::memory_order_relaxed))
            {
                state->cv.wait(lock, [&] { return state->is_disposed.load(std::memory_order_relaxed) || !queue->is_empty(); });

                if (state->is_disposed.load(std::memory_order_relaxed))
                    break;

                if (queue->top()->is_disposed())
                {
                    queue->pop();
                    continue;
                }

                if (const auto now = clock_type::now(); now < queue->top()->get_timepoint())
                {
                    state->cv.wait_for(lock, queue->top()->get_timepoint() - now);
                    continue;
                }

                auto top = queue->pop();

                // we need to keep lock locked due to we have chance to use current_thread during invoking of this schedulable
                if (const auto duration = (*top)())
                {
                    queue->emplace(clock_type::now() + duration.value(), std::move(top));
                }
            }

            queue.reset();
            state->queue_ptr.store(nullptr, std::memory_order_relaxed);
        }

    private:
        std::shared_ptr<state_t> m_state = std::make_shared<state_t>();
        std::thread              m_thread{&data_thread, m_state};
    };

public:
    class worker_strategy
    {
    public:
        worker_strategy() = default;

        template<rpp::constraint::observer TObs, typename... Args, constraint::schedulable_fn<TObs, Args...> Fn>
        void defer_for(duration duration, Fn&& fn, TObs&& obs, Args&&... args) const
        {
            m_state->defer_at(clock_type::now() + duration, std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...);
        }

        rpp::composite_disposable_wrapper get_disposable() const { return rpp::composite_disposable_wrapper{m_state}; }

private:
        std::shared_ptr<disposable> m_state = std::make_shared<disposable>();
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }

};
}