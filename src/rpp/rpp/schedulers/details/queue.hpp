//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/defs.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/schedulers/fwd.hpp>
#include <rpp/utils/utils.hpp>
#include <rpp/utils/tuple.hpp>

#include <condition_variable>
#include <exception>
#include <optional>
#include <queue>
#include <memory>
#include <utility>

namespace rpp::schedulers::details
{
class schedulable_base
{
public:
    explicit schedulable_base(const time_point& time_point) : m_time_point{time_point} {}

    virtual ~schedulable_base() noexcept = default;

    virtual optional_duration operator()()        = 0;
    virtual bool              is_disposed() const = 0;

    time_point get_timepoint() const { return m_time_point; }
    void       set_timepoint(const time_point& timepoint) { m_time_point = timepoint; }

    const std::shared_ptr<schedulable_base>& get_next() const { return m_next; }
    void set_next(std::shared_ptr<schedulable_base>&& next) { m_next = std::move(next); }

    void update_next(std::shared_ptr<schedulable_base>&& next)
    {
        if (next)
            next->set_next(std::move(m_next));
        m_next = std::move(next);
    }

private:
    std::shared_ptr<schedulable_base> m_next{};
    time_point                        m_time_point;
};

template<rpp::constraint::decayed_type Fn, rpp::constraint::observer TObs, rpp::constraint::decayed_type... Args>
    requires constraint::schedulable_fn<Fn, TObs, Args...>
class specific_schedulable final : public schedulable_base
{
public:
    template<rpp::constraint::decayed_same_as<Fn> TFn, rpp::constraint::decayed_same_as<TObs> TTObs, typename... TArgs>
    explicit specific_schedulable(const time_point& time_point, TFn&& in_fn, TTObs&& in_obs, TArgs&&... in_args)
        : schedulable_base{time_point}
        , m_observer{std::forward<TTObs>(in_obs)}
        , m_args(std::forward<TArgs>(in_args)...)
        , m_fn{std::forward<TFn>(in_fn)}
    {
    }

    optional_duration operator()() override
    {
        try
        {
            return m_args.apply(m_fn, m_observer);
        }
        catch(...)
        {
            m_observer.on_error(std::current_exception());
            return std::nullopt;
        }
    }
    bool is_disposed() const override { return m_observer.is_disposed(); }

private:
    RPP_NO_UNIQUE_ADDRESS TObs                       m_observer;
    RPP_NO_UNIQUE_ADDRESS rpp::utils::tuple<Args...> m_args;
    RPP_NO_UNIQUE_ADDRESS Fn                         m_fn;
};

class schedulables_queue
{
public:
    template<rpp::constraint::observer TObs, typename... Args, constraint::schedulable_fn<TObs, Args...> Fn>
    void emplace(const time_point& timepoint, Fn&& fn, TObs&& obs, Args&&... args)
    {
        using schedulable_type = specific_schedulable<std::decay_t<Fn>, std::decay_t<TObs>, std::decay_t<Args>...>;

        emplace_impl(std::make_shared<schedulable_type>(timepoint, std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...));
    }

    void emplace(const time_point& timepoint, std::shared_ptr<schedulable_base>&& schedulable)
    {
        if (!schedulable)
            return;

        schedulable->set_timepoint(timepoint);
        emplace_impl(std::move(schedulable));
    }

    bool is_empty() const { return !m_head; }

    std::shared_ptr<schedulable_base> pop()
    {
        return std::exchange(m_head, m_head->get_next());
    }

    const std::shared_ptr<schedulable_base>& top() const
    {
        return m_head;
    }

private:
    void emplace_impl(std::shared_ptr<schedulable_base>&& schedulable)
    {
        if (!m_head || schedulable->get_timepoint() < m_head->get_timepoint())
        {
            schedulable->set_next(std::move(m_head));
            m_head = std::move(schedulable);
            return;
        }

        schedulable_base* current = m_head.get();
        while (const auto& next = current->get_next())
        {
            if (schedulable->get_timepoint() < next->get_timepoint())
                break;
            current = next.get();
        }

        current->update_next(std::move(schedulable));
    }
private:
    std::shared_ptr<schedulable_base> m_head{};
};
}