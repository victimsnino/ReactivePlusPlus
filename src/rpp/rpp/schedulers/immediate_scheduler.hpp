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
    class worker
    {
    public:
        worker(const rpp::subscription_base& sub)
            : m_sub{sub} {}

        void schedule(constraint::schedulable_fn auto&& fn) const
        {
            schedule(now(), std::forward<decltype(fn)>(fn));
        }

        void schedule(duration delay, constraint::schedulable_fn auto&& fn) const
        {
            schedule(now() + delay, std::forward<decltype(fn)>(fn));
        }

        void schedule(time_point time_point, constraint::schedulable_fn auto&& fn) const
        {
            while (m_sub.is_subscribed())
            {
                std::this_thread::sleep_until(time_point);

                if (!m_sub.is_subscribed())
                    return;

                if (auto duration = fn())
                    time_point = std::max(now(), time_point + duration.value());
                else
                    return;
            }
        }

    private:
        static time_point now() { return clock_type::now();  }

    private:
        rpp::subscription_base m_sub;
    };

    static worker create_worker(const rpp::subscription_base& sub = {})
    {
        return worker{sub};
    }
};
} // namespace rpp::schedulers
