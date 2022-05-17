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

#include <rpp/schedulers/fwd.hpp>
#include <rpp/schedulers/details/worker.hpp>
#include <rpp/subscriptions/subscription_base.hpp>

#include <chrono>
#include <concepts>
#include <thread>

namespace rpp::schedulers
{
/**
 * \brief immediately calls provided schedulable or waits for time_point (in the caller-thread)
 */
class immediate final : public details::scheduler_tag
{
public:
    class worker_strategy
    {
    public:
        worker_strategy(const rpp::subscription_base& sub)
            : m_sub{sub} {}

        void defer_at(time_point time_point, std::invocable auto&& fn) const
        {
            if (!m_sub.is_subscribed())
                return;

            std::this_thread::sleep_until(time_point);

            if (m_sub.is_subscribed())
                fn();
        }

    private:
        rpp::subscription_base m_sub;
    };

    static worker<worker_strategy> create_worker(const rpp::subscription_base& sub = {})
    {
        return worker<worker_strategy>{sub};
    }
};
} // namespace rpp::schedulers
