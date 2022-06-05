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

#include <rpp/operators/details/combining_utils.hpp>

#include <atomic>
#include <memory>

IMPLEMENTATION_FILE(switch_on_next_tag);

namespace rpp::details
{
struct switch_on_next_state_t
{
    std::atomic_size_t          count_of_on_completed{};
    rpp::composite_subscription current_inner_observable = rpp::composite_subscription::empty();
};

auto on_new_observable_switch(std::shared_ptr<switch_on_next_state_t> state)
{
    auto count_of_on_completed = std::shared_ptr<std::atomic_size_t>(state, &state->count_of_on_completed);

    auto on_completed = [=](const constraint::subscriber auto& sub)
    {
        if ((*count_of_on_completed) == 1) // 1 because decrement happens in composite_subscription_callback
            sub.on_completed();
    };

    return [=]<constraint::observable TObs>(TObs&& new_observable, const constraint::subscriber auto& sub)
    {
        using ValueType = utils::extract_observable_type_t<TObs>;

        state->current_inner_observable.unsubscribe();
        state->current_inner_observable = sub.get_subscription().make_child();
        state->current_inner_observable.add([count_of_on_completed, to_remove = state->current_inner_observable, remove_from = sub.get_subscription()]
                                            {
                                                remove_from.remove(to_remove);
                                                --(*count_of_on_completed);
                                            });


        std::forward<TObs>(new_observable).subscribe(combining::create_proxy_subscriber<ValueType>(state->current_inner_observable,
                                                                                                   sub,
                                                                                                   count_of_on_completed,
                                                                                                   forwarding_on_next{},
                                                                                                   forwarding_on_error{},
                                                                                                   on_completed));
    };
}

template<constraint::decayed_type Type>
auto switch_on_next_impl()
{
    using ValueType = utils::extract_observable_type_t<Type>;

    return []<constraint::subscriber_of_type<ValueType> TSub>(TSub&& subscriber)
    {
        auto state = std::make_shared<switch_on_next_state_t>();
        auto count_of_on_completed = std::shared_ptr<std::atomic_size_t>(state, &state->count_of_on_completed);

        return combining::create_proxy_subscriber<Type>(std::forward<TSub>(subscriber),
                                                        count_of_on_completed,
                                                        on_new_observable_switch(state),
                                                        forwarding_on_error{},
                                                        [=](const constraint::subscriber auto& sub)
        {
            if (--(*count_of_on_completed) == 0) 
                sub.on_completed();
        });
    };
}
} // namespace rpp::details
