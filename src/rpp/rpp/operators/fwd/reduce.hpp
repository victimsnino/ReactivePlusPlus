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

template<typename T, typename CastBeforeDrop>
concept is_can_be_averaged = requires(T t, CastBeforeDrop nt)
{
    { t + t } -> std::convertible_to<T>;
    { nt / size_t{} };
};

template<constraint::decayed_type Type, constraint::decayed_type Seed, reduce_accumulator<Seed, Type> AccumulatorFn, std::invocable<Seed> ResultSelectorFn>
struct reduce_impl;

template<constraint::decayed_type CastBeforeDivide, constraint::observable TObs>
auto average_impl(TObs&& observable);

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
     * \param initial_seed initial value for seed which will be applied for first value from observable. Then it will be replaced with result and etc. 
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
    template<typename Seed, reduce_accumulator<Seed, Type> AccumulatorFn, std::invocable<Seed> ResultSelectorFn = std::identity>
    auto reduce(Seed&& initial_seed, AccumulatorFn&& accumulator, ResultSelectorFn&& result_selector = {}) const & requires is_header_included<reduce_tag, Seed, AccumulatorFn, ResultSelectorFn>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<utils::decayed_invoke_result_t<ResultSelectorFn, std::decay_t<Seed>>>(
            reduce_impl<Type, std::decay_t<Seed>, std::decay_t<AccumulatorFn>, std::decay_t<ResultSelectorFn>>{
                                         std::forward<Seed>(initial_seed),
                                         std::forward<AccumulatorFn>(accumulator),
                                         std::forward<ResultSelectorFn>(result_selector)});
    }

   template<typename Seed, reduce_accumulator<Seed, Type> AccumulatorFn, std::invocable<Seed> ResultSelectorFn = std::identity>
    auto reduce(Seed&& initial_seed, AccumulatorFn&& accumulator, ResultSelectorFn&& result_selector = {}) && requires is_header_included<reduce_tag, Seed, AccumulatorFn, ResultSelectorFn>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<utils::decayed_invoke_result_t<ResultSelectorFn, std::decay_t<Seed>>>(
            reduce_impl<Type, std::decay_t<Seed>, std::decay_t<AccumulatorFn>, std::decay_t<ResultSelectorFn>>{
                                         std::forward<Seed>(initial_seed),
                                         std::forward<AccumulatorFn>(accumulator),
                                         std::forward<ResultSelectorFn>(result_selector)});
    }

    /**
     * \brief Calculated the average of emissions and emit final value
     * 
     * \marble reduce
        {
            source observable  : +--1-2-3-|
            operator "average" : +--------2|
        }
     *
     * \return new specific_observable with the average operator as most recent operator.
     * \warning #include <rpp/operators/reduce.hpp>
     * 
     * \par Example
     * \snippet reduce.cpp average
	 *
     * \ingroup transforming_operators
     * \see https://reactivex.io/documentation/operators/average.html
     */
    template<typename CastBeforeDivide = Type, typename ...Args>
    auto average() const & requires (is_header_included<reduce_tag, CastBeforeDivide, Args...> && is_can_be_averaged<Type, CastBeforeDivide>)
    {
        return average_impl<CastBeforeDivide>(*static_cast<const SpecificObservable*>(this));
    }

   template<typename CastBeforeDivide = Type, typename ...Args>
    auto average() && requires (is_header_included<reduce_tag, CastBeforeDivide, Args...> && is_can_be_averaged<Type, CastBeforeDivide>)
    {
        return average_impl<CastBeforeDivide>(std::move(*static_cast<SpecificObservable*>(this)));
    }
};
} // namespace rpp::details
