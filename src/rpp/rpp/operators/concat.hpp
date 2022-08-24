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
struct concat_state_t : public std::enable_shared_from_this<concat_state_t<ValueType>>
{
    concat_state_t(rpp::composite_subscription source_subscription)
        : m_source_subscription{std::move(source_subscription)} {}

    auto get_on_new_observable()
    {
        return[state = this->shared_from_this()]<constraint::observable TObs, constraint::subscriber TSub>(TObs&& new_observable, const TSub & sub)
        {
            if(state->m_inner_subscribed.exchange(true, std::memory_order::acq_rel))
            {
                std::lock_guard lock{ state->m_mutex };
                if (state->m_inner_subscribed.exchange(true, std::memory_order_relaxed))
                {
                    state->m_observables_to_subscribe.push(std::forward<TObs>(new_observable).as_dynamic());
                    return;
                }
            }
            state->subscribe_inner_subscriber(new_observable, sub);
        };
    }
    auto get_on_observable_completed() const
    {
        return [state = this->shared_from_this()](const constraint::subscriber auto& sub)
        {
            if (!state->m_inner_subscribed.load(std::memory_order::acquire))
                sub.on_completed();
        };
    }


private:
    void subscribe_inner_subscriber(const auto& observable, const constraint::subscriber auto& subscriber)
    {
        observable.subscribe(create_subscriber_with_state<ValueType>(subscriber.get_subscription().make_child(),
                                                                     utils::forwarding_on_next{},
                                                                     utils::forwarding_on_error{},
                                                                     [state = this->shared_from_this()](const constraint::subscriber auto& sub)
                                                                     {
                                                                         {
                                                                             std::unique_lock lock{ state->m_mutex };
                                                                             if (!state->m_observables_to_subscribe.empty())
                                                                             {
                                                                                 auto res = std::move(state->m_observables_to_subscribe.front());
                                                                                 state->m_observables_to_subscribe.pop();
                                                                                 lock.unlock();
                                                                                 state->subscribe_inner_subscriber(res, sub);
                                                                                 return;
                                                                             }
                                                                             if (state->m_source_subscription.is_subscribed())
                                                                             {
                                                                                 state->m_inner_subscribed.store(false, std::memory_order_relaxed);
                                                                                 return;
                                                                             }
                                                                         }
                                                                         sub.on_completed();
                                                                     },
                                                                     subscriber));
    }


private:
    const rpp::composite_subscription              m_source_subscription;
    std::mutex                                     m_mutex{};
    std::queue<rpp::dynamic_observable<ValueType>> m_observables_to_subscribe{};
    std::atomic_bool                               m_inner_subscribed{};
};

template<constraint::decayed_type Type>
struct concat_impl
{
    using ValueType = utils::extract_observable_type_t<Type>;

    template<constraint::subscriber_of_type<ValueType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto source_subscription = subscriber.get_subscription().make_child();

        const auto state = std::make_shared<concat_state_t<ValueType>>(source_subscription);

        return create_subscriber_with_state<Type>(std::move(source_subscription),
                                                  state->get_on_new_observable(),
                                                  utils::forwarding_on_error{},
                                                  state->get_on_observable_completed(),
                                                  std::forward<TSub>(subscriber));
    }
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> ... TObservables>
auto concat_with_impl(TObservables&&... observables)
{
    return rpp::source::just(std::forward<TObservables>(observables).as_dynamic()...).concat();
}
} // namespace rpp::details
