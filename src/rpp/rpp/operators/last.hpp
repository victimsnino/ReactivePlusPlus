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

#include <optional>
#include <cstddef>

namespace rpp::operators::details
{
template<rpp::constraint::decayed_type Type>
struct last_observer_strategy
{
    mutable std::optional<Type> value{};

    template<typename T>
    void on_next(const rpp::constraint::observer auto&, T&& v) const
    {
        value.emplace(std::forward<T>(v));
    }

    void on_completed(const rpp::constraint::observer auto& obs) const
    {
        if (value.has_value())
        {
            obs.on_next(std::move(value).value());
            obs.on_completed();
        }
        else
            obs.on_error(std::make_exception_ptr(utils::not_enough_emissions{"last() operator expects at least one emission from observable before completion"}));
    }

    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_set_upstream_strategy set_upstream{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};
    constexpr static empty_on_subscribe on_subscribe{};
};

struct last_t : public operators::details::template_operator_observable_strategy<last_observer_strategy>
{
    template<rpp::constraint::decayed_type T>
    using ResultValue = T;
};
}

namespace rpp::operators
{
/**
 * @brief Emit only the last item provided before on_completed.
 *
 * @marble last
     {
         source observable   : +--1--2--3--|
         operator "last"     : +--3-|
     }
 *
 * @details Actually this operator just updates `std::optional` on every new emission and emits this value on_completed
 * @throws rpp::utils::not_enough_emissions in case of on_completed obtained without any emissions
 *
 * @par Performance notes:
 * - No any heap allocations
 * - No replace std::optional with each new emission and move value from optional on_completed
 *
 * @warning #include <rpp/operators/last.hpp>
 *
 * @par Example:
 * @snippet last.cpp last
 * @snippet last.cpp last empty
 *
 * @ingroup filtering_operators
 * @see https://reactivex.io/documentation/operators/last.html
 */
inline auto last()
{
    return details::last_t{};
}
} // namespace rpp::operators