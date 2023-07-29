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
struct skip_observer_strategy
{
    using DisposableStrategy = rpp::details::none_disposable_strategy;

    mutable size_t count{};

    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        if (count == 0)
            obs.on_next(std::forward<T>(v));
        else
            --count;
    }

    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_on_completed_strategy on_completed{};
    constexpr static forwarding_set_upstream_strategy set_upstream{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};
    constexpr static empty_on_subscribe on_subscribe{};

};

struct skip_t : public operators::details::not_template_operator_observable_strategy<skip_observer_strategy, size_t>
{
    template<rpp::constraint::decayed_type T>
    using ResultValue = T;
};
}

namespace rpp::operators
{
/**
 * @brief Skip first `count` items provided by observable then send rest items as expected
 *
 * @marble skip
 {
     source observable  : +--1-2-3-4-5-6-|
     operator "skip(3)" : +--------4-5-6-|
 }
 *
 * @details Actually this operator just decrements counter and starts to forward emissions when counter reaches zero.
 *
 * @par Performance notes:
 * - No any heap allocations
 * - No any copies/moves just forwarding of emission
 * - Just simple `size_t` decrementing
 *
 * @param count amount of items to be skipped
 * @warning #include <rpp/operators/skip.hpp>
 *
 * @par Example:
 * @snippet skip.cpp skip
 *
 * @ingroup filtering_operators
 * @see https://reactivex.io/documentation/operators/skip.html
 */
inline auto skip(size_t count)
{
    return details::skip_t{count};
}
} // namespace rpp::operators