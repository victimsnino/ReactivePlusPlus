//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once
#include <rpp/observers/state_observer.h>
#include <rpp/subscribers/specific_subscriber.h>
#include <rpp/utils/constraints.h>

namespace rpp
{
class composite_subscription;
}

namespace rpp::details
{
template<constraint::decayed_type Type, typename OnNext, typename OnError, typename OnCompleted>
auto create_subscriber_with_state(auto&&        state,
                                  OnNext&&      on_next,
                                  OnError&&     on_error,
                                  OnCompleted&& on_completed)
{
    return specific_subscriber<Type, state_observer<Type,
                                                    std::decay_t<decltype(state)>,
                                                    std::decay_t<OnNext>,
                                                    std::decay_t<OnError>,
                                                    std::decay_t<OnCompleted>>>
    {
        std::forward<decltype(state)>(state),
        on_next,
        on_error,
        on_completed
    };
}

template<constraint::decayed_type Type, typename OnNext, typename OnError, typename OnCompleted>
auto create_subscriber_with_state(constraint::decayed_same_as<composite_subscription> auto&& sub,
                                  auto&&                                                     state,
                                  OnNext&&                                                   on_next,
                                  OnError&&                                                  on_error,
                                  OnCompleted&&                                              on_completed)
{
    return specific_subscriber<Type, state_observer<Type,
                                                    std::decay_t<decltype(state)>,
                                                    std::decay_t<OnNext>,
                                                    std::decay_t<OnError>,
                                                    std::decay_t<OnCompleted>>>
    {
        std::forward<decltype(sub)>(sub),
        std::forward<decltype(state)>(state),
        on_next,
        on_error,
        on_completed
    };
}
} // namespace rpp::details
