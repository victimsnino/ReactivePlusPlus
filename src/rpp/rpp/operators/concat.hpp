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
#include <rpp/observers/state_observer.hpp>

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
        return[state = this->shared_from_this()]<constraint::observable TObs>(TObs&& new_observable, const constraint::subscriber auto & sub)
        {
            {
                std::lock_guard lock{ state->m_mutex };
                if (std::exchange(state->m_inner_subscribed, true))
                {
                    state->m_observables_to_subscribe.push(std::forward<TObs>(new_observable).as_dynamic());
                    return;
                }
            }
            state->subscribe_inner_subscriber(new_observable, sub);
        };
    }
    auto get_on_observable_completed()
    {
        return [state = this->shared_from_this()](const constraint::subscriber auto& sub)
        {
            {
                std::lock_guard lock{ state->m_mutex };
                if (state->m_inner_subscribed)
                    return;
            }
            sub.on_completed();
        };
    }


private:
    void subscribe_inner_subscriber(const auto& observable, const constraint::subscriber auto& subscriber)
    {
        auto sub = subscriber.get_subscription().make_child();
        observable.subscribe(create_subscriber_with_state<ValueType>(std::move(sub),
                                                  std::forward<decltype(subscriber)>(subscriber),
                                                  forwarding_on_next{},
                                                  forwarding_on_error{},
                                                  [state = this->shared_from_this()](const constraint::subscriber auto& sub)
                                                  {
                                                      {
                                                          std::lock_guard lock{ state->m_mutex };
                                                          if (!state->m_observables_to_subscribe.empty())
                                                          {
                                                              auto res = std::move(state->m_observables_to_subscribe.front());
                                                              state->m_observables_to_subscribe.pop();
                                                              state->subscribe_inner_subscriber(res, sub);
                                                              return;
                                                          }
                                                          if (state->m_source_subscription.is_subscribed())
                                                          {
                                                              state->m_inner_subscribed = false;
                                                              return;
                                                          }
                                                      }
                                                    sub.on_completed();
                                                  }));
    }


private:
    const rpp::composite_subscription              m_source_subscription;
    std::mutex                                     m_mutex{};
    std::queue<rpp::dynamic_observable<ValueType>> m_observables_to_subscribe{};
    bool                                           m_inner_subscribed{};
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
                                             std::forward<TSub>(subscriber),
                                             state->get_on_new_observable(),
                                             forwarding_on_error{},
                                             state->get_on_observable_completed());
    }
};
} // namespace rpp::details
