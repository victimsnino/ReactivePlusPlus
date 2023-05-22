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

#include <rpp/schedulers/current_thread.hpp>
#include <rpp/disposables/base_disposable.hpp>

#include <atomic>
#include <condition_variable>
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
    class disposable final : public base_disposable
    {
    public:
        disposable() {
            // wait until thread is initialized
            std::lock_guard lock{m_queue_mutex};
            if (!m_queue_ptr)
                throw std::logic_error{"Queue was not initialized unexpectedly"};
        }

        ~disposable() override
        {
            dispose_impl();
        }

        void dispose_impl() override
        {
            m_is_disposed.test_and_set(std::memory_order_relaxed);
            m_cv.notify_one();
            m_thread.join();
        }

        template<rpp::constraint::observer TObs, typename... Args, constraint::schedulable_fn<TObs, Args...> Fn>
        void defer_at(time_point time_point, Fn&& fn, TObs&& obs, Args&&... args)
        {
            {
                std::lock_guard lock{m_queue_mutex};
                if (m_queue_ptr)
                    m_queue_ptr->emplace(time_point, std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...);
            }
            m_cv.notify_one();
        }

        private:
            static void data_thread(std::atomic_flag&                       is_disposed,
                                    std::unique_lock<std::recursive_mutex> lock,
                                    std::condition_variable_any&           cv,
                                    details::schedulables_queue*&          queue_ptr)
            {
                queue_ptr = &current_thread::s_queue.emplace();

                while (!is_disposed.test(std::memory_order_relaxed))
                {
                    cv.wait(lock, [&] { return is_disposed.test(std::memory_order_relaxed) || !queue_ptr->is_empty(); });

                    if (is_disposed.test(std::memory_order_relaxed))
                        break;

                    if (!cv.wait_until(lock,
                                       queue_ptr->top()->get_timepoint(),
                                       [&]
                                       {
                                           return is_disposed.test(std::memory_order_relaxed) || queue_ptr->top()->get_timepoint() <= clock_type::now();
                                       }))
                        continue;

                    auto top = queue_ptr->pop();

                    // we need to keep lock locked due to we have chance to use current_thread during invoking of this schedulable
                    if (const auto duration = (*top)()) {
                        queue_ptr->emplace(clock_type::now() + duration.value(), std::move(top));
                    }
                }

                current_thread::s_queue.reset();
                queue_ptr = nullptr;
            }

        private:
            // recursive due to we need to keep lock during invoking of schedulable, but there is chance of recursive scheduling
            std::recursive_mutex         m_queue_mutex{};
            details::schedulables_queue* m_queue_ptr{};
            std::condition_variable_any  m_cv{};
            std::atomic_flag              m_is_disposed{};

            std::thread m_thread{data_thread, m_is_disposed, std::unique_lock{m_queue_mutex}, m_cv, m_queue_ptr};
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

        rpp::disposable_wrapper get_disposable() const { return rpp::disposable_wrapper{m_state}; }

    private:
        std::shared_ptr<disposable> m_state = std::make_shared<disposable>();
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }

};
}