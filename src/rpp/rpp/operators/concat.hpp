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

#include <rpp/subscribers/constraints.hpp>
#include <rpp/operators/fwd/concat.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/observables/dynamic_observable.hpp>

#include <rpp/subscriptions/composite_subscription.hpp>
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state

#include <rpp/sources/just.hpp>
#include <rpp/utils/functors.hpp>

#include <mutex>
#include <memory>
#include <queue>


IMPLEMENTATION_FILE(concat_tag);

namespace rpp::details
{
template<constraint::decayed_type ValueType>
struct concat_state : early_unsubscribe_state
{
    concat_state(const composite_subscription& subscription_of_subscriber)
        : early_unsubscribe_state{subscription_of_subscriber}
        , source_subscription{childs_subscriptions.make_child()} {}

    std::mutex                                mutex{};
    composite_subscription                    source_subscription;
    std::mutex                                queue_mutex{};
    std::queue<dynamic_observable<ValueType>> observables_to_subscribe{};
    std::atomic_bool                          inner_subscribed{};
};

using concat_on_next_inner = merge_forwarding_on_next;
using concat_on_error      = merge_on_error;

struct concat_on_next_outer
{
    template<constraint::decayed_type ValueType, constraint::observable TObs, constraint::subscriber TSub>
    void operator()(TObs&&                                          new_observable,
                    const TSub&                                     sub,
                    const std::shared_ptr<concat_state<ValueType>>& state) const
    {
        if (state->inner_subscribed.exchange(true, std::memory_order::acq_rel))
        {
            std::lock_guard lock{state->queue_mutex};
            if (state->inner_subscribed.exchange(true, std::memory_order::relaxed))
            {
                state->observables_to_subscribe.push(std::forward<TObs>(new_observable).as_dynamic());
                return;
            }
        }
        subscribe_inner_subscriber(new_observable, sub, state);
    }
private:
    template<constraint::decayed_type ValueType>
    static void subscribe_inner_subscriber(const auto&                                     observable,
                                           const constraint::subscriber auto&              subscriber,
                                           const std::shared_ptr<concat_state<ValueType>>& state)
    {
        observable.subscribe(create_subscriber_with_state<ValueType>(
            state->childs_subscriptions.make_child(),
            concat_on_next_inner{},
            concat_on_error{},
            [](const constraint::subscriber auto& sub, const std::shared_ptr<concat_state<ValueType>>& state)
            {
                {
                    std::unique_lock lock{state->queue_mutex};
                    if (!state->observables_to_subscribe.empty())
                    {
                        auto res = std::move(state->observables_to_subscribe.front());
                        state->observables_to_subscribe.pop();
                        lock.unlock();
                        subscribe_inner_subscriber(res, sub, state);
                        return;
                    }
                    if (state->source_subscription.is_subscribed())
                    {
                        state->inner_subscribed.store(false, std::memory_order::relaxed);
                        return;
                    }
                }
                sub.on_completed();
            },
            subscriber,
            state));
    }
};


struct concat_on_completed
{
    template<constraint::decayed_type ValueType>
    void operator()(const constraint::subscriber auto&              sub,
                    const std::shared_ptr<concat_state<ValueType>>& state) const
    {
        std::unique_lock lock{state->queue_mutex};
        if (!state->inner_subscribed.load(std::memory_order::relaxed))
            sub.on_completed();
    }
};

template<constraint::decayed_type Type>
struct concat_impl
{
    using ValueType = utils::extract_observable_type_t<Type>;

    template<constraint::subscriber_of_type<ValueType> TSub>
    auto operator()(TSub&& subscriber) const
    {

        auto state = std::make_shared<concat_state<ValueType>>(subscriber.get_subscription());

        return create_subscriber_with_state<Type>(state->source_subscription,
                                                  concat_on_next_outer{},
                                                  concat_on_error{},
                                                  concat_on_completed{},
                                                  std::forward<TSub>(subscriber),
                                                  std::move(state));
    }
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> ... TObservables>
auto concat_with_impl(TObservables&&... observables)
{
    return source::just(std::forward<TObservables>(observables).as_dynamic()...).concat();
}
} // namespace rpp::details
