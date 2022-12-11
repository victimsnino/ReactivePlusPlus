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

#include <rpp/schedulers/constraints.hpp>
#include <rpp/observables/details/member_overload.hpp>

namespace rpp::details
{
struct debounce_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct debounce_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, debounce_tag>
{
    /**
     * \brief Only emit emission if specified period of time has passed without any other emission. On each new emission timer reset.
     * 
     * \marble debounce
        {
            source    observable   : +--1-2-----3---|
            operator "debounce(4)" : +--------2-----3|
        }
     * \param period is duration of time should be passed since emission from original observable without any new emissions to emit this emission.
     * \param scheduler is scheduler used to run timer for debounce
     * \return new specific_observable with the debounce operator as most recent operator.
     * \warning #include <rpp/operators/debounce.hpp>
     * 
     * \par Example
     * \snippet debounce.cpp debounce
	 *
     * \ingroup utility_operators
     * \see https://reactivex.io/documentation/operators/debounce.html
     */
    template<schedulers::constraint::scheduler TScheduler>
    auto debounce(schedulers::duration period,const TScheduler& scheduler = TScheduler{}) const & requires is_header_included<debounce_tag, TScheduler>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(debounce_impl<Type, TScheduler>{period, scheduler});
    }

    template<schedulers::constraint::scheduler TScheduler>
    auto debounce(schedulers::duration period, const TScheduler& scheduler = TScheduler{}) && requires is_header_included<debounce_tag, TScheduler>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(debounce_impl<Type, TScheduler>{period, scheduler});
    }
};
} // namespace rpp::details
