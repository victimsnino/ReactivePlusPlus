//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once
#include <rpp/observers/state_observer.hpp>
#include <rpp/subscribers/specific_subscriber.hpp>
#include <rpp/utils/constraints.hpp>

namespace rpp
{
class composite_subscription;
}

namespace rpp::details
{
template<constraint::decayed_type                            Type, 
         typename                                            State,
         std::invocable<Type, State>                         OnNext, 
         std::invocable<std::exception_ptr, State>           OnError, 
         std::invocable<State>                               OnCompleted, 
         constraint::decayed_same_as<composite_subscription> Subscription = composite_subscription>
auto create_subscriber_with_state_impl(State&&        state,
                                       OnNext&&       on_next,
                                       OnError&&      on_error,
                                       OnCompleted&&  on_completed,
                                       Subscription&& sub = composite_subscription{})
{
    return specific_subscriber<Type, state_observer<Type,
                                                    std::decay_t<State>,
                                                    std::decay_t<OnNext>,
                                                    std::decay_t<OnError>,
                                                    std::decay_t<OnCompleted>>>
    {
        std::forward<Subscription>(sub),
        std::forward<State>(state),
        std::forward<OnNext>(on_next),
        std::forward<OnError>(on_error),
        std::forward<OnCompleted>(on_completed)
    };
}

template<constraint::decayed_type                  Type, 
         typename                                  State,
         std::invocable<Type, State>               OnNext, 
         std::invocable<std::exception_ptr, State> OnError, 
         std::invocable<State>                     OnCompleted>
auto create_subscriber_with_state(Type&&        state,
                                  OnNext&&      on_next,
                                  OnError&&     on_error,
                                  OnCompleted&& on_completed)
{
    return create_subscriber_with_state_impl<Type>(std::forward<State>(state),
                                                   std::forward<OnNext>(on_next),
                                                   std::forward<OnError>(on_error),
                                                   std::forward<OnCompleted>(on_completed));
}

template<constraint::decayed_type                            Type, 
         constraint::decayed_same_as<composite_subscription> Subscription,
         typename                                            State,
         std::invocable<Type, State>                         OnNext, 
         std::invocable<std::exception_ptr, State>           OnError, 
         std::invocable<State>                               OnCompleted>
auto create_subscriber_with_state(Subscription&& sub,
                                  State&&        state,
                                  OnNext&&       on_next,
                                  OnError&&      on_error,
                                  OnCompleted&&  on_completed)
{
    return create_subscriber_with_state_impl<Type>(std::forward<State>(state),
                                                   std::forward<OnNext>(on_next),
                                                   std::forward<OnError>(on_error),
                                                   std::forward<OnCompleted>(on_completed),
                                                   std::forward<Subscription>(sub));
}
} // namespace rpp::details
