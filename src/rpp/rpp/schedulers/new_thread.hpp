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
            disposable() = default;

            template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, constraint::schedulable_fn<Handler, Args...> Fn>
            void defer_to(time_point time_point, Fn&& fn, Handler&& handler, Args&&... args)
            {
                m_state->queue.emplace(time_point, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
                m_state->has_fresh_data.store(true);
            }

        private:
            void base_dispose_impl(interface_disposable::Mode) noexcept override
            {
                if (!m_thread.joinable())
                    return;

                {
                    std::lock_guard lock{m_state->mutex};
                    m_state->is_disposed = true;
                }
                m_state->cv.notify_all();

                if (m_thread.get_id() != std::this_thread::get_id())
                    m_thread.join();
                else
                    m_thread.detach();
            }

            struct state_t : public details::shared_queue_data
            {
                details::schedulables_queue<current_thread::worker_strategy> queue{};
                bool                                                         is_disposed{};
                std::atomic_bool                                             has_fresh_data{false};
            };

            static void data_thread(std::shared_ptr<state_t> state)
            {
                current_thread::get_queue() = &state->queue;

                while (true)
                {
                    std::unique_lock lock{state->mutex};
                    if (state->queue.is_empty() && state->is_disposed)
                        break;

                    state->cv.wait(lock, [&] { return !state->queue.is_empty() || state->is_disposed; });

                    if (state->queue.is_empty())
                        break;

                    if (state->queue.top()->is_disposed())
                    {
                        state->queue.pop();
                        continue;
                    }

                    if (details::s_last_now_time < state->queue.top()->get_timepoint())
                    {
                        if (const auto now = worker_strategy::now(); now < state->queue.top()->get_timepoint())
                        {
                            state->cv.wait_for(lock, state->queue.top()->get_timepoint() - now, [&] { return state->queue.top()->is_disposed() || worker_strategy::now() >= state->queue.top()->get_timepoint(); });
                            continue;
                        }
                    }

                    auto top = state->queue.pop();
                    state->has_fresh_data.store(!state->queue.is_empty());
                    lock.unlock();

                    while (true)
                    {
                        if (const auto res = top->make_advanced_call())
                        {
                            if (!top->is_disposed())
                            {
                                if (res->can_run_immediately() && !state->has_fresh_data.load())
                                    continue;

                                state->queue.emplace(top->handle_advanced_call(res.value()), std::move(top));
                            }
                        }
                        break;
                    }
                }

                current_thread::get_queue() = nullptr;
            }

        private:
            std::shared_ptr<state_t> m_state = std::make_shared<state_t>();

            RPP_CALL_DURING_CONSTRUCTION(m_state->queue = details::schedulables_queue<current_thread::worker_strategy>(m_state));

            std::thread m_thread{&data_thread, m_state};
        };

    public:
        class worker_strategy
        {
        public:
            worker_strategy() = default;

            template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, constraint::schedulable_fn<Handler, Args...> Fn>
            void defer_to(time_point tp, Fn&& fn, Handler&& handler, Args&&... args) const
            {
                m_state.lock()->defer_to(tp, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
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
