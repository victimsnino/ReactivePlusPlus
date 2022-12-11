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

#include <rpp/operators/lift.hpp>          // required due to operator uses lift
#include <rpp/operators/details/early_unsubscribe.hpp>
#include <rpp/operators/details/serialized_subscriber.hpp>
#include <rpp/operators/details/subscriber_with_state.hpp>
#include <rpp/operators/fwd/debounce.hpp>
#include <rpp/subscribers/constraints.hpp>

#include <rpp/utils/spinlock.hpp>

#include <type_traits>
#include <variant>

IMPLEMENTATION_FILE(debounce_tag);

namespace rpp::details
{
template<typename T, typename Scheduler>
class debounce_state : public early_unsubscribe_state
{
public:
    debounce_state(schedulers::duration period, const Scheduler& scheduler, const composite_subscription& subscription_of_subscriber)
        : early_unsubscribe_state(subscription_of_subscriber)
        , m_period{period}
        , m_worker{scheduler.create_worker(children_subscriptions)} {}

    std::optional<schedulers::time_point> emplace_safe(auto&& v)
    {
        std::lock_guard lock{m_mutex};
        m_value_to_be_emitted.emplace(std::forward<decltype(v)>(v));
        const bool need_to_scheduled = !m_time_when_value_should_be_emitted.has_value() || !m_value_to_be_emitted.has_value();
        m_time_when_value_should_be_emitted = m_worker.now() + m_period;
        return need_to_scheduled ? m_time_when_value_should_be_emitted : std::optional<schedulers::time_point>{};
    }

    std::variant<std::monostate, T, schedulers::duration> extract_value_or_time()
    {
        std::lock_guard lock{m_mutex};
        if (!m_time_when_value_should_be_emitted.has_value() || !m_value_to_be_emitted.has_value())
            return std::monostate{};

        const auto now = m_worker.now();
        if (m_time_when_value_should_be_emitted > now)
            return m_time_when_value_should_be_emitted.value() - now;

        m_time_when_value_should_be_emitted.reset();
        auto v = std::move(m_value_to_be_emitted).value();
        m_value_to_be_emitted.reset();
        return v;
    }

    std::optional<T> extract_value()
    {
        std::lock_guard lock{m_mutex};
        std::optional<T> res{};
        m_value_to_be_emitted.swap(res);
        return res;
    }

    using Worker = decltype(std::declval<Scheduler>().create_worker(std::declval<composite_subscription>()));
    const Worker& get_worker() const {return m_worker;}

private:

    schedulers::duration                  m_period;
    Worker                                m_worker;
    std::mutex                            m_mutex{};
    std::optional<schedulers::time_point> m_time_when_value_should_be_emitted{};
    std::optional<T>                      m_value_to_be_emitted{};
};

struct debounce_on_next
{
    template<typename Value>
    void operator()(Value&& v, const auto& subscriber, const auto& state_ptr) const
    {
        if (const auto time_to_schedule = state_ptr->emplace_safe(std::forward<Value>(v)))
        {
            state_ptr->get_worker().schedule(time_to_schedule.value(),
                                             [=]() mutable -> schedulers::optional_duration
                                             {
                                                 auto value_or_duration = state_ptr->extract_value_or_time();
                                                 if (auto* duration = std::get_if<schedulers::duration>(&value_or_duration))
                                                     return *duration;

                                                 if (auto* value = std::get_if<std::decay_t<Value>>(&value_or_duration))
                                                     subscriber.on_next(std::move(*value));

                                                 return std::nullopt;
                                             });
        }
    }
};

using debounce_on_error = early_unsubscribe_on_error;

struct debounce_on_completed
{
    void operator()(const auto& subscriber, const auto& state_ptr) const
    {
        state_ptr->children_subscriptions.unsubscribe();

        if (auto v = state_ptr->extract_value())
            subscriber.on_next(std::move(v.value()));

        subscriber.on_completed();
    }
};

template<typename T, typename Scheduler>
struct debounce_state_with_serialized_spinlock : debounce_state<T, Scheduler>
{
    using debounce_state<T, Scheduler>::debounce_state;

    // spinlock because most part of time there is only one thread would be active
    utils::spinlock spinlock{};
};

template<constraint::decayed_type Type,schedulers::constraint::scheduler TScheduler>
struct debounce_impl
{
    schedulers::duration period;
    TScheduler           scheduler;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& in_subscriber) const
    {
        auto state = std::make_shared<debounce_state_with_serialized_spinlock<Type, TScheduler>>(period, scheduler, in_subscriber.get_subscription());
        // change subscriber to serialized to avoid manual using of mutex
        auto subscriber = make_serialized_subscriber(std::forward<TSub>(in_subscriber), std::shared_ptr<utils::spinlock>{state, &state->spinlock});

        return create_subscriber_with_state<Type>(state->children_subscriptions,
                                                  debounce_on_next{},
                                                  debounce_on_error{},
                                                  debounce_on_completed{},
                                                  std::move(subscriber),
                                                  std::move(state));
    }
};
} // namespace rpp::details
