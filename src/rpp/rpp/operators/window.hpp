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

#include <rpp/operators/fwd/window.hpp>
#include <rpp/subjects/publish_subject.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state


IMPLEMENTATION_FILE(window_tag);

namespace rpp
{
template<constraint::decayed_type Type>
using windowed_observable = decltype(std::declval<subjects::publish_subject<Type>>().get_observable());
}

namespace rpp::details
{
struct none_lock
{
    static void lock(){}
    static void unlock(){}
};

template<constraint::decayed_type Type, typename Synchronization>
struct window_state
{
    const std::optional<size_t>                            window_size{};
    mutable size_t                                         items_in_current_window{};
    mutable std::optional<subjects::publish_subject<Type>> subject{};
    mutable Synchronization                                synchronization{};
};

struct window_on_next
{
    template<constraint::decayed_type Type, typename Synchronization>
    void operator()(auto&& value, const auto& subscriber, const window_state<Type,Synchronization>& state) const
    {
        std::lock_guard lock{state.synchronization};
        // need to send new subject due to NEW item appeared (we avoid sending new subjects if no any new items)
        if (!state.subject.has_value())
        {
            state.subject = rpp::subjects::publish_subject<Type>{};
            subscriber.on_next(state.subject->get_observable());
            state.items_in_current_window = 0;
        }

        state.subject->get_subscriber().on_next(std::forward<decltype(value)>(value));

        if (!state.window_size.has_value())
            return;

        ++state.items_in_current_window;

        // cleanup current subject, but don't send due to wait for new value
        if (state.items_in_current_window == state.window_size.value())
        {
            state.subject->get_subscriber().on_completed();
            state.subject.reset();
        }
    }
};

struct window_on_error
{
    template<constraint::decayed_type Type, typename Synchronization>
    void operator()(const std::exception_ptr& err, const auto& subscriber, const window_state<Type, Synchronization>& state) const
    {
        std::lock_guard lock{state.synchronization};
        state.subject.get_subscriber().on_error(err);
        subscriber.on_error(err);
    }
};

struct window_on_completed
{
    template<constraint::decayed_type Type, typename Synchronization>
    void operator()(const auto& subscriber, const window_state<Type, Synchronization>& state) const
    {
        std::lock_guard lock{state.synchronization};

        state.subject.get_subscriber().on_completed();
        subscriber.on_completed();
    }
};

template<constraint::decayed_type Type>
struct window_lift_impl
{
    size_t window_size{};

    template<constraint::subscriber_of_type<windowed_observable<Type>> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();

        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          window_on_next{},
                                                          window_on_error{},
                                                          window_on_completed{},
                                                          std::forward<TSub>(subscriber),
                                                          window_state<Type, none_lock>(window_size));
    }
};

template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct window_with_time_lift_impl
{
    std::optional<size_t> window_size{};
    schedulers::duration  period{};
    TScheduler            scheduler{};

    template<constraint::subscriber_of_type<windowed_observable<Type>> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<window_state<Type, std::mutex>>(std::nullopt);
        auto subscription = subscriber.get_subscription();

        scheduler.create_worker(subscription).schedule([period=period, weak_state = std::weak_ptr{state}]
        {
            if (auto state = weak_state.lock())
            {
                std::lock_guard lock{state->synchronization};
                if (state->subject.has_value())
                {
                    state->subject.get_subscriber().on_completed();
                    state->subject.reset();
                }
            }
            return period;
        });

        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  window_on_next{},
                                                  window_on_error{},
                                                  window_on_completed{},
                                                  std::forward<TSub>(subscriber),
                                                  std::move(state));
    }
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs, schedulers::constraint::scheduler TScheduler>
auto window_with_time_impl(TObs&& obs, rpp::schedulers::duration period, const TScheduler& scheduler)
{
    return std::forward<TObs>(obs).template lift<windowed_observable<Type>>(window_with_time_lift_impl<Type, TScheduler>{period, scheduler});
    
}
} // namespace rpp::details
