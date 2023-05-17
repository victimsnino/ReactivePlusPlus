//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/operators/fwd.hpp>
#include <rpp/sources/concat.hpp>
#include <rpp/utils/utils.hpp>

#include <cstddef>
#include <type_traits>

namespace rpp::operators::details
{
struct repeat_t
{
    size_t count;

    template<rpp::constraint::observable TObservable>
    auto operator()(TObservable&& observable) const
    {
        return rpp::source::concat(utils::repeated_container{std::forward<TObservable>(observable), count});
    }
};
}

namespace rpp::operators
{
/**
 * @brief Re-subscribes on current observable provided amount of times when `on_completed` obtained
 *
 * @marble repeat
     {
         source observable    : +-1-2-3-|
         operator "repeat(2)" : +-1-2-3-1-2-3-|
     }
 *
 * @details Actually this operator does concat(obs, obs, obs....)
 *
 * @param count total amount of times subscription happens. For example:
 *  - `repeat(0)`  - means no any subscription at all
 *  - `repeat(1)`  - behave like ordinal observable
 *  - `repeat(10)` - 1 normal subscription and 9 re-subscriptions during on_completed
 * @return new base_observable with the repeat operator as most recent operator.
 * @warning #include <rpp/operators/repeat.hpp>
 *
 * @par Examples:
 * @snippet repeat.cpp repeat
 *
 * @ingroup utility_operators
 * @see https://reactivex.io/documentation/operators/repeat.html
 */
inline auto repeat(size_t count)
{
    return details::repeat_t{count};
}
} // namespace rpp::operators