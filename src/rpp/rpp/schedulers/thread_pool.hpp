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

#include <rpp/schedulers/new_thread.hpp>

namespace rpp::schedulers
{
    /**
     * @brief Scheduler owning static thread pool of workers and using "some" thread from this pool on `create_worker` call
     * @warning Expected to use this scheduler as local variable to share same threads between different operators or as static variable
     *
     * @par Examples
     * @snippet thread_pool.cpp thread_pool
     *
     * @ingroup schedulers
     */
    class thread_pool final
    {
        using original_worker = decltype(new_thread::create_worker());

        class worker_strategy
        {
        public:
            worker_strategy(const original_worker& original_worker)
                : m_original_worker{original_worker}
            {
            }

            template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, constraint::schedulable_fn<Handler, Args...> Fn>
            void defer_for(duration duration, Fn&& fn, Handler&& handler, Args&&... args) const
            {
                m_original_worker.schedule(duration, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
            }

            template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, constraint::schedulable_fn<Handler, Args...> Fn>
            void defer_to(time_point tp, Fn&& fn, Handler&& handler, Args&&... args) const
            {
                m_original_worker.schedule(tp, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
            }

            static constexpr rpp::schedulers::details::none_disposable get_disposable() { return {}; }
            static rpp::schedulers::time_point                         now() { return original_worker::now(); }

        private:
            original_worker m_original_worker;
        };

    public:
        explicit thread_pool(size_t threads_count = std::thread::hardware_concurrency())
            : m_state{std::make_shared<state>(threads_count)}
        {
        }

        rpp::schedulers::worker<worker_strategy> create_worker() const
        {
            return rpp::schedulers::worker<worker_strategy>{m_state->get()};
        }

    private:
        class state
        {
        public:
            explicit state(size_t threads_count)
            {
                threads_count = std::max(size_t{1}, threads_count);
                workers.reserve(threads_count);
                for (size_t i = 0; i < threads_count; ++i)
                    workers.emplace_back(new_thread::create_worker());
            }

            ~state()
            {
                for (const auto& worker : workers)
                    worker.get_disposable().dispose();
            }

            const original_worker& get() { return workers[index++ % workers.size()]; }

        private:
            std::vector<original_worker> workers{};
            size_t                       index{};
        };

        std::shared_ptr<state> m_state{};
    };
} // namespace rpp::schedulers
