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
concept scan_accumulator = reduce_accumulator<Fn,Result,Type>;

template<constraint::decayed_type Type, constraint::decayed_type Result, scan_accumulator<Result, Type> AccumulatorFn>
struct scan_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, scan_tag>
{
   /**
    * \brief Apply accumulator function for each emission from observable and result of accumulator from previous step and emit (and cache) resulting value
    * 
    * \marble scan
    {
        source observable                : +--1-2-3-|
        operator "scan: s=1, (s,x)=>s+x" : +--2-4-7-|
    }
    *
    * \details Acttually this operator applies provided accumulator function to seed and new emission, emits resulting value and updates seed value for next emission
    *
    * \param initial_value initial value for seed which will be applied for first value from observable (instead of emitting this as first value). Then it will be replaced with result and etc. 
    * \param accumulator function which accepts seed value and new value from observable and return new value of seed. Can accept seed by move-reference.
    *
    * \return new specific_observable with the scan operator as most recent operator.
    * \warning #include <rpp/operators/scan.hpp>
    * 
    * \par Example
    * \snippet scan.cpp scan
    * \snippet scan.cpp scan_vector
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to store seed
    * - <b>OnNext</b>
    *    - Applies accumulator to each emission
    *    - Updates seed value
    *    - Emits new seed value
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - Just forwards original on_completed
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
