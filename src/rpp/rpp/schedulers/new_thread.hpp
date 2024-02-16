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
                while (!m_state->queue_ptr.load(std::memory_order::seq_cst))
                {
                };
            }

            ~disposable() override
            {
                if (!m_thread.joinable())
                    return;

                // just notify
                m_state->is_destroying.store(true, std::memory_order::seq_cst);
                m_state->cv.notify_all();
                m_thread.detach();
            }

            template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, constraint::schedulable_fn<Handler, Args...> Fn>
            void defer_at(time_point time_point, Fn&& fn, Handler&& handler, Args&&... args)
            {
                if (is_disposed())
                    return;

                std::lock_guard lock{m_state->mutex};
                // guarded by lock
                if (const auto queue = m_state->queue_ptr.load(std::memory_order::seq_cst))
                    queue->emplace(time_point, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
            }

        private:
            void base_dispose_impl(interface_disposable::Mode) noexcept override
            {
                if (!m_thread.joinable())
                    return;

                // just need atomicity, not guarding anything
                m_state->is_disposed.store(true, std::memory_order::seq_cst);
                m_state->cv.notify_all();

                if (m_thread.get_id() != std::this_thread::get_id())
                    m_thread.join();
                else
                    m_thread.detach();
            }

            struct state_t : public details::shared_queue_data
            {
                std::atomic<details::schedulables_queue<current_thread::worker_strategy>*> queue_ptr{};
                std::atomic_bool                                                           is_disposed{};
                std::atomic_bool                                                           is_destroying{};
            };

            static void data_thread(std::shared_ptr<state_t> state)
            {
                auto& queue = current_thread::s_queue;
                state->queue_ptr.store(&queue.emplace(state), std::memory_order::seq_cst);

                while (!state->is_disposed.load(std::memory_order::seq_cst))
                {
                    std::unique_lock lock{state->mutex};

                    if (state->is_destroying.load(std::memory_order::seq_cst) && queue->is_empty())
                        break;

                    state->cv.wait(lock, [&] { return state->is_disposed.load(std::memory_order::seq_cst) || !queue->is_empty() || state->is_destroying.load(std::memory_order::seq_cst); });

                    if (state->is_disposed.load(std::memory_order::seq_cst) || state->is_destroying.load(std::memory_order::seq_cst))
                        break;

                    if (queue->top()->is_disposed())
                    {
                        queue->pop();
                        continue;
                    }

                    if (details::s_last_now_time < queue->top()->get_timepoint())
                    {
                        if (const auto now = worker_strategy::now(); now < queue->top()->get_timepoint())
                        {
                            state->cv.wait_for(lock, queue->top()->get_timepoint() - now, [&] { return state->is_disposed.load(std::memory_order::seq_cst) || state->is_destroying.load(std::memory_order::seq_cst) || worker_strategy::now() >= queue->top()->get_timepoint(); });
                            continue;
                        }
                    }

                    auto top = queue->pop();
                    lock.unlock();

                    if (const auto timepoint = (*top)())
                        queue->emplace(timepoint.value(), std::move(top));
                }

                std::unique_lock lock{state->mutex};
                state->queue_ptr.store(nullptr, std::memory_order::seq_cst);
                queue.reset();
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
                m_state.lock()->defer_at(now() + duration, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
            }

            rpp::disposable_wrapper get_disposable() const { return m_state; }

            static rpp::schedulers::time_point now() { return details::now(); }

        private:
            disposable_wrapper_impl<disposable> m_state = disposable_wrapper_impl<disposable>::make();
        };

        static rpp::schedulers::worker<worker_strategy> create_worker()
        {
            return rpp::schedulers::worker<worker_strategy>{};
        }
    };
} // namespace rpp::schedulers
