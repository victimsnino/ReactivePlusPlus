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
#include <rpp/schedulers/constraints.hpp>
#include <rpp/schedulers/trampoline_scheduler.hpp>

namespace rpp::details
{
struct sample_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, schedulers::constraint::scheduler_not_trampoline TScheduler>
struct sample_with_time_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, sample_tag>
{
    /**
     * \brief Emit most recent emitted from original observable emission obtained during last period of time.
     * \details Emit item immediately in case of completion of the original observable
     * 
     * \marble sample_with_time
        {
            source observable              : +--1---2-3-4---5-6-7-|
            operator "sample_with_time(2)" : +--1---2---4---5---7-|
        }
     *
     * \param period sampling period
     * \scheduler scheduler to use to schedule emissions with provided sampling period
     * \return new specific_observable with the sample_with_time operator as most recent operator.
     * \warning #include <rpp/operators/sample.hpp>
     * 
     * \par Example
     * \snippet sample.cpp sample_with_time
	 *
     * \ingroup filtering_operators
     * \see https://reactivex.io/documentation/operators/sample.htmlhttps://reactivex.io/documentation/operators/sample.html
     */
    template<schedulers::constraint::scheduler_not_trampoline TScheduler, typename ...Args>
    auto sample_with_time(schedulers::duration period, const TScheduler& scheduler) const & requires is_header_included<sample_tag, TScheduler, Args...>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(sample_with_time_impl<Type, TScheduler>{period, scheduler});
    }

    template<schedulers::constraint::scheduler_not_trampoline TScheduler, typename ...Args>
    auto sample_with_time(schedulers::duration period, const TScheduler& scheduler) && requires is_header_included<sample_tag, TScheduler, Args...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(sample_with_time_impl<Type, TScheduler>{period, scheduler});
    }
};
} // namespace rpp::details
