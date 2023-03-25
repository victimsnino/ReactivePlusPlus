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

#include <rpp/utils/constraints.hpp>

#include <exception>

namespace rpp::constraint
{
template<typename S, typename Type>
concept observer_strategy = requires(const S& strategy, const Type& v)
{
    strategy.on_next(v);
    strategy.on_next(Type{});
    strategy.on_error(std::exception_ptr{});
    strategy.on_completed();
};
}

namespace rpp::details
{
template<constraint::decayed_type Type>
class dynamic_strategy;

template<constraint::decayed_type Type, std::invocable<Type> OnNext,  std::invocable<std::exception_ptr> OnError, std::invocable<> OnCompleted>
struct lambda_strategy;
} // namespace rpp::details

namespace rpp
{
template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
class base_observer;

template<constraint::decayed_type Type>
using dynamic_observer = base_observer<Type, details::dynamic_strategy<Type>>;

template<constraint::decayed_type Type, std::invocable<Type> OnNext,  std::invocable<std::exception_ptr> OnError, std::invocable<> OnCompleted>
using lambda_observer = base_observer<Type, details::lambda_strategy<Type, OnNext, OnError, OnCompleted>>;

template<constraint::decayed_type Type,
         std::invocable<Type> OnNext,
         std::invocable<std::exception_ptr> OnError,
         std::invocable<> OnCompleted>
auto make_lambda_observer(OnNext&&      on_next,
                          OnError&&     on_error,
                          OnCompleted&& on_completed) -> lambda_observer<Type,
                                                                         std::decay_t<OnNext>,
                                                                         std::decay_t<OnError>,
                                                                         std::decay_t<OnCompleted>>;
} // namespace rpp
