//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observers/observer.hpp>
#include <rpp/observers/details/observer_storage.hpp>
#include <rpp/utils/function_traits.hpp>

#include <rpp/defs.hpp>

namespace rpp::details::observers
{
template<constraint::decayed_type Type,
         std::invocable<Type> OnNext,
         std::invocable<const std::exception_ptr&> OnError,
         std::invocable<> OnCompleted>
struct lambda_strategy
{
    lambda_strategy(auto&& in_on_next, auto&& in_on_error, auto&& in_on_completed)
        : on_next(std::forward<decltype(in_on_next)>(in_on_next))
        , on_error(std::forward<decltype(in_on_error)>(in_on_error))
        , on_completed(std::forward<decltype(in_on_completed)>(in_on_completed))
        {}

    RPP_NO_UNIQUE_ADDRESS OnNext      on_next;
    RPP_NO_UNIQUE_ADDRESS OnError     on_error;
    RPP_NO_UNIQUE_ADDRESS OnCompleted on_completed;

    static void set_upstream(const disposable_wrapper&) noexcept {}
    static bool is_disposed() noexcept { return false; }
};
}

namespace rpp
{
template<constraint::decayed_type Type,
         std::invocable<Type> OnNext,
         std::invocable<const std::exception_ptr&> OnError,
         std::invocable<> OnCompleted>
auto make_lambda_observer(OnNext&&      on_next,
                          OnError&&     on_error,
                          OnCompleted&& on_completed) -> lambda_observer<Type,
                                                                         std::decay_t<OnNext>,
                                                                         std::decay_t<OnError>,
                                                                         std::decay_t<OnCompleted>>
{
    return lambda_observer<Type,
                           std::decay_t<OnNext>,
                           std::decay_t<OnError>,
                           std::decay_t<OnCompleted>>{details::observers::construct_with<details::observers::lambda_strategy<Type, std::decay_t<OnNext>, std::decay_t<OnError>, std::decay_t<OnCompleted>>>{},
                                                      std::forward<OnNext>(on_next),
                                                      std::forward<OnError>(on_error),
                                                      std::forward<OnCompleted>(on_completed)};
}

template<constraint::decayed_type Type,
         std::invocable<Type> OnNext,
         std::invocable<const std::exception_ptr&> OnError,
         std::invocable<> OnCompleted>
auto make_lambda_observer(const rpp::disposable_wrapper& d,
                          OnNext&&      on_next,
                          OnError&&     on_error,
                          OnCompleted&& on_completed) -> lambda_observer_with_disposable<Type,
                                                                                         std::decay_t<OnNext>,
                                                                                         std::decay_t<OnError>,
                                                                                         std::decay_t<OnCompleted>>
{
    return lambda_observer_with_disposable<Type,
                                           std::decay_t<OnNext>,
                                           std::decay_t<OnError>,
                                           std::decay_t<OnCompleted>>
    {
        d,
        details::observers::construct_with<details::observers::lambda_strategy<Type, std::decay_t<OnNext>, std::decay_t<OnError>, std::decay_t<OnCompleted>>>{},
        std::forward<OnNext>(on_next),
        std::forward<OnError>(on_error),
        std::forward<OnCompleted>(on_completed)
    };
}
} // namespace rpp
