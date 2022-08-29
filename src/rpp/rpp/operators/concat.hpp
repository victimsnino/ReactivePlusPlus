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
struct concat_state
{
    concat_state(subscription_base source_subscription)
        : m_source_subscription{std::move(source_subscription)} {}

    struct on_next
    {
        template<constraint::observable TObs, constraint::subscriber TSub>
        void operator()(TObs&&                                            new_observable,
                        const TSub&                                       sub,
                        const std::shared_ptr<concat_state<ValueType>>& state) const
        {
            if (state->m_inner_subscribed.exchange(true, std::memory_order::acq_rel))
            {
                std::lock_guard lock{state->m_mutex};
                if (state->m_inner_subscribed.exchange(true, std::memory_order::relaxed))
                {
                    state->m_observables_to_subscribe.push(std::forward<TObs>(new_observable).as_dynamic());
                    return;
                }
            }
            subscribe_inner_subscriber(new_observable, sub, state);
        }
    };

    struct on_completed
    {
        void operator()(const constraint::subscriber auto&                sub,
                        const std::shared_ptr<concat_state<ValueType>>& state) const
        {
            std::unique_lock lock{state->m_mutex};
            if (!state->m_inner_subscribed.load(std::memory_order::relaxed))
                sub.on_completed();
        }
    };


private:
    static void subscribe_inner_subscriber(const auto&                                     observable,
                                           const constraint::subscriber auto&              subscriber,
                                           const std::shared_ptr<concat_state<ValueType>>& state)
    {
        observable.subscribe(create_subscriber_with_state<ValueType>(
            subscriber.get_subscription().make_child(),
            utils::forwarding_on_next{},
            utils::forwarding_on_error{},
            [](const constraint::subscriber auto& sub, const std::shared_ptr<concat_state<ValueType>>& state)
            {
                {
                    std::unique_lock lock{state->m_mutex};
                    if (!state->m_observables_to_subscribe.empty())
                    {
                        auto res = std::move(state->m_observables_to_subscribe.front());
                        state->m_observables_to_subscribe.pop();
                        lock.unlock();
                        subscribe_inner_subscriber(res, sub, state);
                        return;
                    }
                    if (state->m_source_subscription.is_subscribed())
                    {
                        state->m_inner_subscribed.store(false, std::memory_order::relaxed);
                        return;
                    }
                }
                sub.on_completed();
            },
            subscriber,
            state));
    }


private:
    const subscription_base                   m_source_subscription;
    std::mutex                                m_mutex{};
    std::queue<dynamic_observable<ValueType>> m_observables_to_subscribe{};
    std::atomic_bool                          m_inner_subscribed{};
};

template<constraint::decayed_type Type>
struct concat_impl
{
    using ValueType = utils::extract_observable_type_t<Type>;

    template<constraint::subscriber_of_type<ValueType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto source_subscription = subscriber.get_subscription().make_child();

        auto state = std::make_shared<concat_state<ValueType>>(source_subscription);

        return create_subscriber_with_state<Type>(std::move(source_subscription),
                                                  typename concat_state<ValueType>::on_next{},
                                                  utils::forwarding_on_error{},
                                                  typename concat_state<ValueType>::on_completed{},
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
