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
template<rpp::constraint::observer TObserver>
struct skip_observer_strategy
{
    using DisposableStrategyToUseWithThis = rpp::details::none_disposable_strategy;

    RPP_NO_UNIQUE_ADDRESS TObserver observer;
    mutable size_t                  count{};

    template<typename T>
    void on_next(T&& v) const
    {
        if (count == 0)
            observer.on_next(std::forward<T>(v));
        else
            --count;
    }

    void on_error(const std::exception_ptr& err) const { observer.on_error(err); }
    void on_completed() const                          { observer.on_completed(); }

    void set_upstream(const disposable_wrapper& d)     { observer.set_upstream(d); }
    bool is_disposed() const                           { return observer.is_disposed(); }
};

struct skip_t : public operators::details::operator_observable_strategy_diffferent_types<skip_observer_strategy, rpp::utils::types<>, size_t>
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