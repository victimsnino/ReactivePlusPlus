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

#include <rpp/observables/details/member_overload.hpp>

namespace rpp::details
{
struct scan_tag;
}

namespace rpp::details
{
// accepts Result and Type and returns Result
template<typename Fn, typename Result, typename Type>
concept scan_accumulator = std::is_invocable_r_v<std::decay_t<Result>, Fn, std::decay_t<Result>, std::decay_t<Type>>;

template<constraint::decayed_type Type, constraint::decayed_type Result, scan_accumulator<Result, Type> AccumulatorFn>
struct scan_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, scan_tag>
{
    /**
     * \brief 
     * 
     * \marble scan
        {
            
        }
     *
     * \return new specific_observable with the scan operator as most recent operator.
     * \warning #include <rpp/operators/scan.hpp>
     * 
     * \par Example
     * \snippet scan.cpp scan
	 *
     * \ingroup transforming_operators
     * \see https://reactivex.io/documentation/operators/scan.html
     */
    template<typename Result, scan_accumulator<Result, Type> AccumulatorFn>
    auto scan(Result&& initial_value, AccumulatorFn&& accumulator) const & requires is_header_included<scan_tag, Result, AccumulatorFn>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<std::decay_t<Result>>(scan_impl<Type, std::decay_t<Result>, std::decay_t<AccumulatorFn>>{std::forward<Result>(initial_value), std::forward<AccumulatorFn>(accumulator)});
    }

    template<typename Result, scan_accumulator<Result, Type> AccumulatorFn>
    auto scan(Result&& initial_value, AccumulatorFn&& accumulator) && requires is_header_included<scan_tag, Result, AccumulatorFn>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<std::decay_t<Result>>(scan_impl<Type, std::decay_t<Result>, std::decay_t<AccumulatorFn>>{std::forward<Result>(initial_value), std::forward<AccumulatorFn>(accumulator)});
    }
};
} // namespace rpp::details
