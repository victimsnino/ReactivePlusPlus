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
#include <rpp/operators/fwd/ref_count.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/sources/create.hpp>

IMPLEMENTATION_FILE(ref_count_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto ref_count_impl(TObs&& observable)
{
    struct state_t
    {
        size_t                 count_of_active_subs{};
        composite_subscription sub = composite_subscription::empty();
        std::mutex             mutex{};
    };
    return source::create<Type>([observable = std::forward<TObs>(observable), state = std::make_shared<state_t>()](const constraint::subscriber_of_type<Type> auto& subscriber)
    {
        bool need_to_connect = false;
        {
            std::lock_guard lock{state->mutex};
            need_to_connect = ++state->count_of_active_subs == 1;
            if (need_to_connect)
                state->sub = composite_subscription{};
        }

        subscriber.get_subscription().add([state = state]
        {
            std::lock_guard lock{state->mutex};
            if (--state->count_of_active_subs == 0)
                state->sub.unsubscribe();
        });

        observable.subscribe(subscriber);
        if (need_to_connect)
            observable.connect(state->sub);
    });
}
} // namespace rpp::details
