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

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/switch_on_next.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/observers/state_observer.hpp>
#include <rpp/utils/functors.hpp>

#include <rpp/operators/details/combining_utils.hpp>

#include <atomic>
#include <memory>

IMPLEMENTATION_FILE(switch_on_next_tag);

namespace rpp::details
{
struct switch_on_next_state_t : public std::enable_shared_from_this<switch_on_next_state_t>
{
    auto on_new_observable_switch()
    {
        auto state        = shared_from_this();
        auto on_completed = [=](const constraint::subscriber auto& sub)
        {
            if (state->count_of_on_completed.load(std::memory_order::acquire) == 1) // 1 because decrement happens in composite_subscription_callback
                sub.on_completed();
        };

        return [=]<constraint::observable TObs>(const TObs& new_observable, const constraint::subscriber auto& sub)
        {
            using ValueType = utils::extract_observable_type_t<TObs>;

            state->current_inner_observable.unsubscribe();
            state->current_inner_observable = sub.get_subscription().make_child();
            state->current_inner_observable.add([state = std::weak_ptr{state}, remove_from = sub.get_subscription()]
            {
                if (auto locked = state.lock())
                    locked->count_of_on_completed.fetch_sub(1, std::memory_order::relaxed);
            });


            new_observable.subscribe(combining::create_proxy_subscriber<ValueType>(state->current_inner_observable,
                                                                                   sub,
                                                                                   state->count_of_on_completed,
                                                                                   utils::forwarding_on_next{},
                                                                                   utils::forwarding_on_error{},
                                                                                   on_completed));
        };
    }

    std::atomic_size_t          count_of_on_completed{};
private:
    rpp::composite_subscription current_inner_observable = rpp::composite_subscription::empty();
};


template<constraint::decayed_type Type>
struct switch_on_next_impl
{
    using ValueType = utils::extract_observable_type_t<Type>;

    template<constraint::subscriber_of_type<ValueType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        const auto state = std::make_shared<switch_on_next_state_t>();

        return combining::create_proxy_subscriber<Type>(std::forward<TSub>(subscriber),
                                                        state->count_of_on_completed,
                                                        state->on_new_observable_switch(),
                                                        utils::forwarding_on_error{},
                                                        [=](const constraint::subscriber auto& sub)
        {
            if (state->count_of_on_completed.fetch_sub(1, std::memory_order::acq_rel) == 1)
                sub.on_completed();
        });
    }
};
} // namespace rpp::details
