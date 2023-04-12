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

#include <memory>
#include <utility>

namespace rpp::schedulers::details
{
class schedulable_base
{
public:
    schedulable_base(const time_point time_point, std::shared_ptr<schedulable_base> next)
        : m_time_point{time_point}
        , m_next{std::move(next)}
        {}

    void set_next(const std::shared_ptr<schedulable_base>& next)
    {
        m_next = next;
    }

    const std::shared_ptr<schedulable_base>& get_next() const
    {
        return m_next;
    }

    time_point get_timepoint() const
    {
        return m_time_point;
    }

private:
    time_point       m_time_point;
    std::shared_ptr<schedulable_base> m_next{};
};

class schedulables_shared_queue
{
public:
    template<rpp::constraint::observer TObs, typename...Args>
    void push(const time_point time_point, constraint::schedulable_fn<TObs, Args...> auto&& fn, TObs&& obs, Args&&...args)
    {
        if (obs.is_disposed())
            return;

        if (!m_head || m_head->get_timepoint() > time_point) {
            m_head = std::make_shared<schedulable_base>(time_point, m_head);
            return;
        }

        auto current = m_head.get();
        while(const auto& next = current->get_next()) {
            if (next->get_timepoint() > time_point) {
                return;
            }
            current = next.get();
        }
        current->set_next(std::make_shared<schedulable_base>(time_point, current->get_next()));
    }

    bool empty() const { return !m_head; }

private:
    std::shared_ptr<schedulable_base> m_head{};
};
}