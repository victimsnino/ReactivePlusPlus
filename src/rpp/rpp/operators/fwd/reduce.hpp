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

template<typename T>
concept is_can_be_summed = requires(T t)
{
    { t + t } -> std::convertible_to<T>;
};

template<typename T, typename CastBeforeDrop>
concept is_can_be_averaged = is_can_be_summed<T> && requires(CastBeforeDrop nt)
{
    { nt / size_t{} };
};

template<constraint::decayed_type Type, constraint::decayed_type Seed, reduce_accumulator<Seed, Type> AccumulatorFn, std::invocable<Seed&&> ResultSelectorFn>
struct reduce_impl;

template<constraint::decayed_type CastBeforeDivide, constraint::observable TObs>
auto average_impl(TObs&& observable);

template<constraint::observable TObs>
auto sum_impl(TObs&& observable);

template<constraint::observable TObs>
auto count_impl(TObs&& observable);

template<constraint::observable TObs, typename Comparator>
auto min_impl(TObs&& observable, Comparator&& comparator);

template<constraint::observable TObs, typename Comparator>
auto max_impl(TObs&& observable, Comparator&& comparator);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, reduce_tag>
{
    /**
     * \brief Applies accumulator function to each emission from observable and result of accumulator from previous step and emits final value
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
     * \ingroup aggregate_operators
     * \see https://reactivex.io/documentation/operators/reduce.html
     */
    template<typename Seed, reduce_accumulator<Seed, Type> AccumulatorFn, std::invocable<Seed&&> ResultSelectorFn = std::identity>
    auto reduce(Seed&& initial_seed, AccumulatorFn&& accumulator, ResultSelectorFn&& result_selector = {}) const & requires is_header_included<reduce_tag, Seed, AccumulatorFn, ResultSelectorFn>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<utils::decayed_invoke_result_t<ResultSelectorFn, std::decay_t<Seed>>>(
            reduce_impl<Type, std::decay_t<Seed>, std::decay_t<AccumulatorFn>, std::decay_t<ResultSelectorFn>>{
                                         std::forward<Seed>(initial_seed),
                                         std::forward<AccumulatorFn>(accumulator),
                                         std::forward<ResultSelectorFn>(result_selector)});
    }

    template<typename Seed, reduce_accumulator<Seed, Type> AccumulatorFn, std::invocable<Seed&&> ResultSelectorFn = std::identity>
    auto reduce(Seed&& initial_seed, AccumulatorFn&& accumulator, ResultSelectorFn&& result_selector = {}) && requires is_header_included<reduce_tag, Seed, AccumulatorFn, ResultSelectorFn>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<utils::decayed_invoke_result_t<ResultSelectorFn, std::decay_t<Seed>>>(
            reduce_impl<Type, std::decay_t<Seed>, std::decay_t<AccumulatorFn>, std::decay_t<ResultSelectorFn>>{
                                         std::forward<Seed>(initial_seed),
                                         std::forward<AccumulatorFn>(accumulator),
                                         std::forward<ResultSelectorFn>(result_selector)});
    }

    /**
     * \brief Calculates the average of emissions and emits final value
     * 
     * \marble average
        {
            source observable  : +--1-2-3-|
            operator "average" : +--------2|
        }
     *
     * \tparam CastBeforeDivide cast accumulated value to this type before division
     * \return new specific_observable with the average operator as most recent operator.
     * \throws rpp::utils::not_enough_emissions in case of no any emissions from original observable
     *
     * \warning #include <rpp/operators/reduce.hpp>
     * 
     * \par Example
     * \snippet reduce.cpp average
	 *
     * \ingroup aggregate_operators
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

    
    /**
     * \brief Calculates the sum of emissions and emits final value
     * 
     * \marble sum
        {
            source observable  : +--1-2-3-|
            operator "sum"     : +--------6|
        }
     *
     * \return new specific_observable with the sum operator as most recent operator.
     * \throws rpp::utils::not_enough_emissions in case of no any emissions from original observable
     *
     * \warning #include <rpp/operators/reduce.hpp>
     * 
     * \par Example
     * \snippet reduce.cpp sum
	 *
     * \ingroup aggregate_operators
     * \see https://reactivex.io/documentation/operators/sum.html
     */
    template<typename ...Args>
    auto sum() const & requires (is_header_included<reduce_tag, Args...> && is_can_be_summed<Type>)
    {
        return sum_impl(*static_cast<const SpecificObservable*>(this));
    }

    template<typename ...Args>
    auto sum() && requires (is_header_included<reduce_tag, Args...> && is_can_be_summed<Type>)
    {
        return sum_impl(std::move(*static_cast<SpecificObservable*>(this)));
    }

    /**
     * \brief Calculates the amount of emitted emissions and emits this count
     * 
     * \marble count
        {
            source observable  : +--1-2-3-|
            operator "count"   : +--------3|
        }
     *
     * \return new specific_observable with the count operator as most recent operator.
     * \warning #include <rpp/operators/reduce.hpp>
     * 
     * \par Example
     * \snippet reduce.cpp count
	 *
     * \ingroup aggregate_operators
     * \see https://reactivex.io/documentation/operators/count.html
     */
    template<typename ...Args>
    auto count() const & requires is_header_included<reduce_tag, Args...>
    {
        return count_impl(*static_cast<const SpecificObservable*>(this));
    }

    template<typename ...Args>
    auto count() && requires is_header_included<reduce_tag, Args...>
    {
        return count_impl(std::move(*static_cast<SpecificObservable*>(this)));
    }

    /**
     * \brief Emits the emission which has minimal value from the whole observable
     * 
     * \marble min
        {
            source observable  : +-6-1-2-3-|
            operator "min"     : +---------1|
        }
     *
     * \param comparator is function to deduce if left value is less than right
     * \return new specific_observable with the min operator as most recent operator.
     * \throws rpp::utils::not_enough_emissions in case of no any emissions from original observable
     *
     * \warning #include <rpp/operators/reduce.hpp>
     * 
     * \par Example
     * \snippet reduce.cpp min
	 *
     * \ingroup aggregate_operators
     * \see https://reactivex.io/documentation/operators/min.html
     */
    template<std::strict_weak_order<Type, Type> Comparator = std::less<Type>, typename ...Args>
    auto min(Comparator&& comparator = {}) const & requires is_header_included<reduce_tag, Comparator, Args...>
    {
        return min_impl(*static_cast<const SpecificObservable*>(this), std::forward<Comparator>(comparator));
    }

    template<std::strict_weak_order<Type, Type> Comparator = std::less<Type>, typename ...Args>
    auto min(Comparator&& comparator = {}) && requires is_header_included<reduce_tag, Comparator, Args...>
    {
        return min_impl(std::move(*static_cast<SpecificObservable*>(this)), std::forward<Comparator>(comparator));
    }

    /**
     * \brief Emits the emission which has maximal value from the whole observable
     * 
     * \marble max
        {
            source observable  : +-6-1-2-3-|
            operator "max"     : +---------6|
        }
     *
     * \param comparator is function to deduce if left value is less than right
     * \return new specific_observable with the max operator as most recent operator.
     * \throws rpp::utils::not_enough_emissions in case of no any emissions from original observable
     *
     * \warning #include <rpp/operators/reduce.hpp>
     * 
     * \par Example
     * \snippet reduce.cpp max
	 *
     * \ingroup aggregate_operators
     * \see https://reactivex.io/documentation/operators/max.html
     */
    template<std::strict_weak_order<Type, Type> Comparator = std::less<Type>, typename ...Args>
    auto max(Comparator&& comparator = {}) const & requires is_header_included<reduce_tag, Comparator, Args...>
    {
        return max_impl(*static_cast<const SpecificObservable*>(this), std::forward<Comparator>(comparator));
    }

    template<std::strict_weak_order<Type, Type> Comparator = std::less<Type>, typename ...Args>
    auto max(Comparator&& comparator = {}) && requires is_header_included<reduce_tag, Comparator, Args...>
    {
        return max_impl(std::move(*static_cast<SpecificObservable*>(this)), std::forward<Comparator>(comparator));
    }
};
} // namespace rpp::details
