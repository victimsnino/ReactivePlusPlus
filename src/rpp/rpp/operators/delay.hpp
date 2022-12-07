//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                    TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/defs.hpp>                                    // RPP_NO_UNIQUE_ADDRESS
#include <rpp/operators/lift.hpp>                          // required due to operator uses lift
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/delay.hpp>                     // own forwarding
#include <rpp/subscribers/constraints.hpp>                 // constraint::subscriber_of_type
#include <rpp/utils/overloaded.hpp>

#include <variant>

IMPLEMENTATION_FILE(delay_tag);

namespace rpp::details
{
struct completion {};

template<typename T, typename Subscriber, typename Worker>
class queue_based_worker final : std::enable_shared_from_this<queue_based_worker<T, Subscriber, Worker>>
{
public:
    queue_based_worker(schedulers::duration delay, Worker&& worker, const Subscriber& subscriber)
        : m_delay{delay}
        , m_worker{std::move(worker)}
        , m_subscriber{subscriber} {}

    queue_based_worker(schedulers::duration delay, Worker&& worker, Subscriber&& subscriber)
        : m_delay{delay}
        , m_worker{std::move(worker)}
        , m_subscriber{std::move(subscriber)} {}

    struct on_next
    {
        void operator()(auto&& value, const std::shared_ptr<queue_based_worker<T, Subscriber, Worker>>& state) const
        {
            state->emplace(std::forward<decltype(value)>(value));
        }
    };

    struct on_error
    {
        void operator()(const std::exception_ptr& err, const std::shared_ptr<queue_based_worker<T, Subscriber, Worker>>& state) const
        {
            state->emplace(err);
        }
    };

    struct on_completed
    {
        void operator()(const std::shared_ptr<queue_based_worker<T, Subscriber, Worker>>& state) const
        {
            state->emplace(completion{});
        }
    };

private:
    void emplace(std::variant<T, std::exception_ptr, completion>&& item)
    {
        if (const auto timepoint = emplace_safe(std::move(item)))
        {
            m_worker.schedule(timepoint.value(),
                              [state = this->shared_from_this()]()-> schedulers::optional_duration
                              {
                                  return state->drain_queue();
                              });
        }
    }

    std::optional<schedulers::time_point> emplace_safe(std::variant<T, std::exception_ptr, completion>&& item)
    {
        std::lock_guard lock{m_mutex};
        m_queue.emplace(++m_current_id, m_worker.now()+m_delay, std::move(item));
        if (!m_active && m_queue.size() == 1)
        {
            m_active = true;
            return m_queue.top().time;
        }
        return {};
    }

    schedulers::optional_duration drain_queue()
    {
        const auto now = m_worker.now();
        while (true)
        {
            std::unique_lock lock{m_mutex};
            if (m_queue.empty())
            {
                m_active = false;
                return {};
            }

            auto& top = m_queue.top();
            if (top.time > now)
                return top.time - now;

            auto item = std::move(top.item);
            m_queue.pop();
            lock.unlock();

            std::visit(utils::overloaded
                       {
                           [&](T&&                       v) { m_subscriber.on_next(std::move(v)); },
                           [&](const std::exception_ptr& err) { m_subscriber.on_error(err); },
                           [&](completion) { m_subscriber.on_completed(); }
                       },
                       std::move(item));
        }
    }

private:
    struct emission
    {
        size_t                                          id{};
        schedulers::time_point                          time{};
        std::variant<T, std::exception_ptr, completion> item{};

        bool operator<(const emission& other) const { return std::tie(time, id) >= std::tie(other.time, other.id); }
    };

    schedulers::duration          m_delay;
    Worker                        m_worker;
    Subscriber                    m_subscriber;
    std::mutex                    m_mutex{};
    size_t                        m_current_id{};
    std::priority_queue<emission> m_queue{};
    bool                          m_active{};
};


template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct delay_impl
{
    RPP_NO_UNIQUE_ADDRESS TScheduler scheduler;
    schedulers::duration             delay;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto worker = scheduler.create_worker(subscriber.get_subscription());
        auto subscription = subscriber.get_subscription().make_child();

        using state_t = queue_based_worker<Type, std::decay_t<TSub>, std::decay_t<decltype(worker)>>;
        auto state = std::make_shared<state_t>(delay, std::move(worker), std::forward<TSub>(subscriber));

        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  typename state_t::on_next{},
                                                  typename state_t::on_error{},
                                                  typename state_t::on_completed{},
                                                  std::move(state));
    }
};
} // namespace rpp::details
