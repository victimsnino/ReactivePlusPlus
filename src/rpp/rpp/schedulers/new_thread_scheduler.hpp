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
        using new_thread_schedulable = schedulable_wrapper<worker_strategy>;

        worker_strategy(const rpp::composite_subscription& sub)
        {
            if (!sub.is_subscribed())
                return;

            auto shared = std::make_shared<state>();
            // init while it is alive as shared
            shared->init_thread(sub);
            m_state = shared;
        }

        bool is_subscribed() const
        {
            if (const auto locked = m_state.lock())
                return locked->is_subscribed();
            return false;
        }

        void defer_at(time_point time_point, constraint::schedulable_fn auto&& fn) const
        {
            defer_at(time_point, new_thread_schedulable{*this, time_point, std::forward<decltype(fn)>(fn)});
        }

        void defer_at(time_point time_point, new_thread_schedulable&& fn) const
        {
            if (auto locked = m_state.lock())
                locked->defer_at(time_point, std::move(fn));
        }

        static time_point now() { return clock_type::now(); }

    private:
        class state : public std::enable_shared_from_this<state>
        {
        public:
            state() = default;
            state(const state&) = delete;
            state(state&&) noexcept = delete;

            bool is_subscribed() const { return m_sub->is_subscribed(); }

            void defer_at(time_point time_point, new_thread_schedulable&& fn)
            {
                if (m_sub->is_subscribed())
                    m_queue.emplace(time_point, std::move(fn));
            }

            void init_thread(const rpp::composite_subscription& sub)
            {
                m_thread = std::thread{[state = shared_from_this()]()
                {
                    state->data_thread();
                }};
                const auto callback = rpp::callback_subscription{[state = weak_from_this()]
                {
                    const auto locked = state.lock();
                    if (!locked)
                        return;

                    locked->m_queue.unsubscribe();

                    if (locked->m_thread.joinable() && locked->m_thread.get_id() != std::this_thread::get_id())
                        locked->m_thread.join();
                    else
                        locked->m_thread.detach();
                }};
                sub.add(callback);
                m_sub.reset(callback);
            }

        private:
            void data_thread()
            {
                std::optional<new_thread_schedulable> fn{};
                while (m_queue.is_subscribed())
                {
                    if (m_queue.pop_with_wait(fn))
                    {
                        (*fn)();
                        fn.reset();
                    }
                }

                // clear
                m_queue.unsubscribe();
            }

            details::queue_worker_state<new_thread_schedulable> m_queue{};
            std::thread                                         m_thread{};
            rpp::subscription_guard                             m_sub = rpp::subscription_base::empty();
        };

        // original shared would alive in thread!
        std::weak_ptr<state> m_state{};
    };

    static auto create_worker(const rpp::composite_subscription& sub = composite_subscription{})
    {
        return worker<worker_strategy>{sub};
    }
};
} // namespace rpp::schedulers
