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

#include "rpp/disposables/base_disposable.hpp"
#include <rpp/schedulers/current_thread.hpp>
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
            disposable() 
            {
                m_thread = std::jthread{[](std::unique_lock<std::mutex> lock, std::condition_variable& cv, details::schedulables_queue*& queue_ptr)
                {
                    queue_ptr = &current_thread::s_queue.emplace();
                }, std::unique_lock{m_queue_mutex}};
            }
        private:

            std::mutex                   m_queue_mutex{};
            std::condition_variable      m_cv{};
            details::schedulables_queue* m_queue{};

            std::jthread                 m_thread{};
    };

public:
    class worker_strategy
    {
    public:
        worker_strategy() = default;

        template<rpp::constraint::observer TObs, typename... Args, constraint::schedulable_fn<TObs, Args...> Fn>
        static void defer_for(duration duration, Fn&& fn, TObs&& obs, Args&&... args)
        {
            
        }

        rpp::disposable_wrapper get_disposable() const { return rpp::disposable_wrapper{m_disposable}; }

    private:
        std::shared_ptr<disposable> m_disposable = std::make_shared<disposable>();
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }

};
}