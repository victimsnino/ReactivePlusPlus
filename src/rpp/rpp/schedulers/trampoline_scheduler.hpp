//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                            TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include "rpp/utils/utilities.hpp"

#include <rpp/schedulers/fwd.hpp>                       // own forwarding
#include <rpp/schedulers/details/worker.hpp>            // worker
#include <rpp/subscriptions/composite_subscription.hpp> // lifetime
#include <rpp/schedulers/details/queue_worker_state.hpp>// state

#include <concepts>
#include <chrono>
#include <functional>
#include <thread>


namespace rpp::schedulers
{
/**
 * \brief schedules execution of schedulables via queueing tasks to the caller thread with priority to time_point and order
 *
 * \par Example
 * \snippet trampoline.cpp trampoline
 *
 * \ingroup schedulers
 */
class trampoline final : public details::scheduler_tag
{
    class current_thread_schedulable : public details::schedulable
    {
    public:
        current_thread_schedulable(time_point                  time_point,
                                   std::invocable auto&&       fn,
                                   rpp::composite_subscription subscription)
            : schedulable(time_point, get_thread_local_id(), std::forward<decltype(fn)>(fn))
            , m_subscription{std::move(subscription)} {}

        bool is_subscribed() const { return m_subscription.is_subscribed(); }

    private:
        static size_t get_thread_local_id()
        {
            static thread_local size_t s_id;
            return s_id++;
        }

    private:
        rpp::composite_subscription m_subscription{};
    };

    /**
     * \brief Returns optional thread_local schedulable queue. If optional has value -> someone just owns thread.
     */
    static std::optional<std::priority_queue<current_thread_schedulable>>& get_schedulable_queue()
    {
        static thread_local std::optional<std::priority_queue<current_thread_schedulable>> s_queue{};
        return s_queue;
    }

    static void drain_queue()
    {
        auto& queue          = get_schedulable_queue();
        auto  reset_at_final = utils::finally_action{[] { get_schedulable_queue().reset(); }};

        while (!queue->empty())
        {
            const auto& top = queue->top();

            auto function = wait_and_extract_executable_if_subscribed(top);

            // firstly we need to pop schedulable from queue due to execution of function can add new schedulable
            queue->pop();

            if (function)
                function();
        }
    }

    [[nodiscard]] static std::function<void()> wait_and_extract_executable_if_subscribed(const  current_thread_schedulable& schedulable)
    {
        if (!schedulable.is_subscribed())
            return {};

        std::this_thread::sleep_until(schedulable.GetTimePoint());

        if (!schedulable.is_subscribed())
            return {};

        return std::move(schedulable.ExtractFunction());
    }

public:
    class worker_strategy
    {
    public:
        explicit worker_strategy(const rpp::composite_subscription& subscription)
            : m_sub{subscription} {}

        void defer_at(time_point time_point, std::invocable auto&& fn) const
        {
            if (!m_sub.is_subscribed())
                return;

            auto drain_handle = ensure_queue_if_no_any_owner();
            get_schedulable_queue()->emplace(time_point, std::forward<decltype(fn)>(fn), m_sub);
        }

        static time_point now() { return clock_type::now(); }

    private:
        rpp::composite_subscription m_sub;
    };

    /**
     * \brief Take ownership over queue and return handle which would drain queue during destruction
     */
    [[nodiscard]] static utils::finally_action<void(*)()> ensure_queue_if_no_any_owner()
    {
        auto& queue = get_schedulable_queue();
        if (queue.has_value())
            return utils::finally_action{+[] {}};

        queue = std::priority_queue<current_thread_schedulable>{};
        return utils::finally_action{&drain_queue};
    }

    static auto create_worker(const rpp::composite_subscription& sub = composite_subscription{})
    {
        return worker<worker_strategy>{sub};
    }
};
} // namespace rpp::schedulers
