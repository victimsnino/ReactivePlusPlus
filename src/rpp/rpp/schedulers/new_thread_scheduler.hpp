//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/schedulers/fwd.hpp>                       // own forwarding
#include <rpp/schedulers/details/worker.hpp>            // worker
#include <rpp/subscriptions/composite_subscription.hpp> // lifetime
#include <rpp/subscriptions/subscription_guard.hpp>     // lifetime
#include <rpp/schedulers/details/queue_worker_state.hpp>// state

#include <concepts>
#include <chrono>
#include <functional>
#include <thread>

namespace rpp::schedulers
{
/**
 * \brief scheduler which schedules execution of schedulables via queueing tasks to another thread with priority to time_point and order
 * \warning Creates new thread for each "create_worker" call. Any scheduled task will be queued to created thread for execution with respect to time_point and number of task
 * \ingroup schedulers
 */
class new_thread final : public details::scheduler_tag
{
public:
    class worker_strategy
    {
    public:
        worker_strategy(const rpp::composite_subscription& sub)
            : m_state{std::make_shared<state>()}
        {
            m_state->init_thread(sub);
        }

        void defer_at(time_point time_point, std::invocable auto&& fn) const
        {
            m_state->defer_at(time_point, std::forward<decltype(fn)>(fn));
        }

    private:
        class state : public std::enable_shared_from_this<state>
        {
        public:
            state() = default;
            state(const state&) = delete;
            state(state&&) noexcept = delete;

            void defer_at(time_point time_point, std::invocable auto&& fn)
            {
                if (m_sub->is_subscribed())
                    m_queue.emplace(time_point, std::forward<decltype(fn)>(fn));
            }

            void init_thread(const rpp::composite_subscription& sub)
            {
                auto as_shared = shared_from_this();

                m_thread = std::jthread{[state = as_shared](const std::stop_token& token)
                {
                    state->data_thread(token);
                }};
                m_sub.reset(sub.add([state = as_shared]
                {
                    state->m_thread.request_stop();

                    if (state->m_thread.get_id() != std::this_thread::get_id())
                        state->m_thread.join();
                    else
                        state->m_thread.detach();
                }));
            }

        private:
            void data_thread(const std::stop_token& token)
            {
                std::function<void()> fn{};
                while (!token.stop_requested())
                {
                    if (m_queue.pop_with_wait(fn, token))
                    {
                        fn();
                        fn = {};
                    }
                }

                // clear
                m_queue.reset();
            }

            details::queue_worker_state m_queue{};
            std::jthread                m_thread{};
            rpp::subscription_guard     m_sub = rpp::subscription_base::empty();
        };

        std::shared_ptr<state> m_state{};
    };

    static auto create_worker(const rpp::composite_subscription& sub = composite_subscription{})
    {
        return worker<worker_strategy>{sub};
    }
};
} // namespace rpp::schedulers
