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
struct reduce_tag;
}

namespace rpp::details
{
// accepts Result and Type and returns Result
template<typename Fn, typename Result, typename Type>
concept reduce_accumulator = std::is_invocable_r_v<std::decay_t<Result>, Fn, std::decay_t<Result>, std::decay_t<Type>>;

template<constraint::decayed_type Type, constraint::decayed_type Result, reduce_accumulator<Result, Type> AccumulatorFn>
struct reduce_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, reduce_tag>
{
    /**
     * \brief Apply accumulator function to each emission from observable and result of accumulator from previous step and emit final value
     * 
     * \marble reduce
        {
            source observable                  : +--1-2-3-|
            operator "reduce: s=1, (s,x)=>s+x" : +--------7|
        }
     *
     * \param initial_value initial value for seed which will be applied for first value from observable. Then it will be replaced with result and etc. 
     * \param accumulator function which accepts seed value and new value from observable and return new value of seed. Can accept seed by move-reference.
     *
     * \return new specific_observable with the reduce operator as most recent operator.
     * \warning #include <rpp/operators/reduce.hpp>
     * 
     * \par Example
     * \snippet reduce.cpp reduce
     * \snippet reduce.cpp reduce_vector
	 *
     * \ingroup transforming_operators
     * \see https://reactivex.io/documentation/operators/reduce.html
     */
    template<typename Result, reduce_accumulator<Result, Type> AccumulatorFn>
    auto reduce(Result&& initial_value, AccumulatorFn&& accumulator) const & requires is_header_included<reduce_tag, Result, AccumulatorFn>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<std::decay_t<Result>>(reduce_impl<Type, std::decay_t<Result>, std::decay_t<AccumulatorFn>>{std::forward<Result>(initial_value), std::forward<AccumulatorFn>(accumulator)});
    }

    template<typename Result, reduce_accumulator<Result, Type> AccumulatorFn>
    auto reduce(Result&& initial_value, AccumulatorFn&& accumulator) && requires is_header_included<reduce_tag, Result, AccumulatorFn>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<std::decay_t<Result>>(reduce_impl<Type, std::decay_t<Result>, std::decay_t<AccumulatorFn>>{std::forward<Result>(initial_value), std::forward<AccumulatorFn>(accumulator)});
    }
};
} // namespace rpp::details
