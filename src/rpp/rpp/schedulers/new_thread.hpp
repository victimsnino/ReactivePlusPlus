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

#include <rpp/disposables/details/base_disposable.hpp>
#include <rpp/schedulers/current_thread.hpp>

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
 * @details This scheduler useful when we want to have separate thread for processing starting from some timepoint.
 * @ingroup schedulers
 */
class new_thread
{
    class disposable final : public rpp::details::base_disposable
    {
    public:
        disposable()
        {
            // just waiting
            while (!m_state->queue_ptr.load(std::memory_order::relaxed))
            {
            };
        }

        ~disposable() override
        {
            if (!m_thread.joinable())
                return;

            // just notify
            m_state->is_destroying.store(true, std::memory_order::relaxed);
            m_state->cv.notify_all();
            m_thread.detach();
        }

        template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, constraint::schedulable_fn<Handler, Args...> Fn>
        void defer_at(time_point time_point, Fn&& fn, Handler&& handler, Args&&... args)
        {
            if (is_disposed())
                return;

            std::lock_guard lock{m_state->queue_mutex};
            // guarded by lock
            if (const auto queue = m_state->queue_ptr.load(std::memory_order::relaxed))
                queue->emplace(time_point, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
        }

    private:
        void dispose_impl() noexcept override
        {
            if (!m_thread.joinable())
                return;

            // just need atomicity, not guarding anything
            m_state->is_disposed.store(true, std::memory_order::relaxed);
            m_state->cv.notify_all();

            if (m_thread.get_id() != std::this_thread::get_id())
                m_thread.join();
            else
                m_thread.detach();
        }

        struct state_t
        {
            // recursive due to we need to keep lock during invoking of schedulable, but there is chance of recursive scheduling
            std::recursive_mutex                      queue_mutex{};
            std::atomic<details::schedulables_queue*> queue_ptr{};
            std::condition_variable_any               cv{};
            std::atomic_bool                          is_disposed{};
            std::atomic_bool                          is_destroying{};
        };

        static void data_thread(std::shared_ptr<state_t> state)
        {
            std::unique_lock lock{state->queue_mutex};
            auto&            queue = current_thread::s_queue;
            state->queue_ptr.store(&queue.emplace(std::shared_ptr<std::condition_variable_any>{state, &state->cv}), std::memory_order::relaxed);

            while (!state->is_disposed.load(std::memory_order::relaxed) && (!state->is_destroying.load(std::memory_order::relaxed) || !queue->is_empty()))
            {
                state->cv.wait(lock, [&] { return state->is_disposed.load(std::memory_order::relaxed) || !queue->is_empty() || state->is_destroying.load(std::memory_order::relaxed); });

                if (state->is_disposed.load(std::memory_order::relaxed) || state->is_destroying.load(std::memory_order::relaxed))
                    break;

                if (queue->top()->is_disposed())
                {
                    queue->pop();
                    continue;
                }

                if (current_thread::s_last_now_time < queue->top()->get_timepoint())
                {
                    if (const auto now = worker_strategy::now(); now < queue->top()->get_timepoint())
                    {
                        state->cv.wait_for(lock, queue->top()->get_timepoint() - now, [&] { return state->is_disposed.load(std::memory_order::relaxed) || state->is_destroying.load(std::memory_order::relaxed) || worker_strategy::now() >= queue->top()->get_timepoint(); });
                        continue;
                    }
                }

                auto top = queue->pop();

                // we need to keep lock locked due to we have chance to use current_thread during invoking of this schedulable
                if (const auto duration = (*top)())
                {
                    queue->emplace(worker_strategy::now() + duration.value(), std::move(top));
                }
            }

            queue.reset();
            state->queue_ptr.store(nullptr, std::memory_order::relaxed);
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

        template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, constraint::schedulable_fn<Handler, Args...> Fn>
        void defer_for(duration duration, Fn&& fn, Handler&& handler, Args&&... args) const
        {
            m_state->defer_at(now() + duration, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
        }

        rpp::disposable_wrapper get_disposable() const { return rpp::disposable_wrapper{m_state}; }

        static rpp::schedulers::time_point now() { return current_thread::worker_strategy::now(); }

    private:
        std::shared_ptr<disposable> m_state = std::make_shared<disposable>();
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }
};
}
