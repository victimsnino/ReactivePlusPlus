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

#include <rpp/schedulers/fwd.hpp>                   // own forwarding
#include <rpp/schedulers/details/worker.hpp>        // worker
#include <rpp/subscriptions/subscription_base.hpp>  // lifetime
#include <rpp/schedulers/details/utils.hpp>

#include <chrono>
#include <concepts>
#include <thread>

namespace rpp::schedulers
{
/**
 * \brief immediately calls provided schedulable or waits for time_point (in the caller-thread)
 * \ingroup schedulers
 */
class immediate final : public details::scheduler_tag
{
public:
    class worker_strategy
    {
    public:
        worker_strategy(const rpp::subscription_base& sub)
            : m_sub{sub} {}

        void defer_at(time_point time_point, constraint::schedulable_fn auto&& fn) const
        {
            details::immediate_scheduling_while_condition(time_point, std::forward<decltype(fn)>(fn), m_sub, []{return true;});
        }

        static time_point now() { return clock_type::now();  }
    private:
        rpp::subscription_base m_sub;
    };

    static auto create_worker(const rpp::subscription_base& sub = {})
    {
        return worker<worker_strategy>{sub};
    }
};
} // namespace rpp::schedulers
