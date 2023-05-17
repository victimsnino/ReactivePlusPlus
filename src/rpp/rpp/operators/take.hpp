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
#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <cstddef>

namespace rpp::operators::details
{
struct take_observer_strategy
{
    mutable size_t count{};

    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        if (count > 0)
        {
            --count;
            obs.on_next(std::forward<T>(v));
        }

        if (count == 0)
            obs.on_completed();
    }

    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_on_completed_strategy on_completed{};
    constexpr static forwarding_set_upstream_strategy set_upstream{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};
};

template<rpp::constraint::observable TObservable>
using take_observable = details::identity_operator_observable<std::decay_t<TObservable>, details::take_observer_strategy, size_t>;

struct take_t
{
    size_t count;

    template<rpp::constraint::observable TObservable>
    auto operator()(TObservable&& observable) const
    {
        return take_observable<TObservable>{std::forward<TObservable>(observable), count};
    }
};
}

namespace rpp::operators
{
/**
 * @brief Emit only first `count` items provided by observable, then send `on_completed`
 *
 * @marble take
 {
     source observable  : +--1-2-3-4-5-6-|
     operator "take(3)" : +--1-2-3|
 }
 * @details Actually this operator just emits emissions while counter is not zero and decrements counter on each emission
 *
 * @par Performance notes:
 * - No any heap allocations
 * - No any copies/moves just forwarding of emission
 * - Just simple `size_t` decrementing
 *
 * @param count amount of items to be emitted. 0 - instant complete
 * @warning #include <rpp/operators/take.hpp>
 *
 * @par Example:
 * @snippet take.cpp take
 *
 * @ingroup filtering_operators
 * @see https://reactivex.io/documentation/operators/take.html
 */
inline auto take(size_t count)
{
    return details::take_t{count};
}
} // namespace rpp::operators