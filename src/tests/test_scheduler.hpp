//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/schedulers.hpp>

static rpp::schedulers::time_point s_current_time{ std::chrono::seconds{0} };

class test_scheduler final : public rpp::schedulers::details::scheduler_tag
{
public:
    class worker_strategy
    {
    public:
        worker_strategy(const rpp::subscription_base& sub,
            std::shared_ptr<std::vector<rpp::schedulers::time_point>> schedulings)
            : m_sub{ sub }
            , m_schedulings{ schedulings } {}

        void defer_at(rpp::schedulers::time_point time_point, rpp::schedulers::constraint::schedulable_fn auto&& fn) const
        {
            while (m_sub.is_subscribed())
            {
                m_schedulings->push_back(time_point);

                if (auto duration = fn())
                    time_point = std::max(now(), time_point + duration.value());
                else
                    return;
            }
        }

        static rpp::schedulers::time_point now() { return s_current_time; }

    private:
        rpp::subscription_base                                    m_sub;
        std::shared_ptr<std::vector<rpp::schedulers::time_point>> m_schedulings;
    };

    test_scheduler() {}

    rpp::schedulers::worker<worker_strategy> create_worker(const rpp::subscription_base& sub = {}) const
    {
        return rpp::schedulers::worker<worker_strategy>{sub, m_schedulings};
    }

    const auto& get_schedulings() const { return *m_schedulings; }

private:
    std::shared_ptr<std::vector<rpp::schedulers::time_point>> m_schedulings = std::make_shared<std::vector<rpp::schedulers::time_point>>();
};