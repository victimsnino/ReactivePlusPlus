//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <rpp/observers/base_observer.hpp>
#include <rpp/utils/function_traits.hpp>

namespace rpp::details
{
template<constraint::decayed_type Type,
         std::invocable<Type> OnNext,
         std::invocable<std::exception_ptr> OnError,
         std::invocable<> OnCompleted>
struct lambda_strategy
{
    OnNext      on_next;
    OnError     on_error;
    OnCompleted on_completed;
};
} // namespace rpp::details

namespace rpp
{
template<constraint::decayed_type Type,
         std::invocable<Type> OnNext,
         std::invocable<std::exception_ptr> OnError,
         std::invocable<> OnCompleted>
auto make_lambda_observer(OnNext&&      on_next,
                          OnError&&     on_error,
                          OnCompleted&& on_completed) -> lambda_observer<Type,
                                                                         std::decay_t<OnNext>,
                                                                         std::decay_t<OnError>,
                                                                         std::decay_t<OnCompleted>>
{
    return {
        std::forward<OnNext>(on_next),
        std::forward<OnError>(on_error),
        std::forward<OnCompleted>(on_completed)
    };
}
} // namespace rpp
