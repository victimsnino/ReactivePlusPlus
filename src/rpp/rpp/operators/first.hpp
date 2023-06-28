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
struct first_observer_strategy
{
    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        obs.on_next(std::forward<T>(v));
        obs.on_completed();
    }

    void on_completed(const rpp::constraint::observer auto& obs) const
    {
        obs.on_error(std::make_exception_ptr(utils::not_enough_emissions{"first() operator expects at least one emission from observable before completion"}));
    }


    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_set_upstream_strategy set_upstream{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};
    constexpr static empty_on_subscribe on_subscribe{};

};

struct first_t : public operators::details::not_template_operator_observable_strategy<first_observer_strategy>
{
    template<rpp::constraint::decayed_type T>
    using ResultValue = T;
};
}

namespace rpp::operators
{
/**
 * @brief Emit only the first item.
 *
 * @marble first
     {
         source observable   : +--1--2--3--|
         operator "first"    : +--1|
     }
 *
 * @details Actually this operator is `take(1)` with exception during `on_completed` if no any emision happens. So, it just forwards first obtained emission and emits on_completed immediately
 * @throws rpp::utils::not_enough_emissions in case of on_completed obtained without any emissions
 *
 * @par Performance notes:
 * - No any heap allocations
 * - No any copies/moves just forwarding of emission
 *
 * @warning #include <rpp/operators/first.hpp>
 *
 * @par Example:
 * @snippet first.cpp first
 * @snippet first.cpp first_empty
 *
 * @ingroup filtering_operators
 * @see https://reactivex.io/documentation/operators/first.html
 */
inline auto first()
{
    return details::first_t{};
}
} // namespace rpp::operators