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
struct timeout_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct timeout_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, timeout_tag>
{
    /**
     * \brief 
     * 
     * \marble timeout
        {
            
        }
     *
     * \return new specific_observable with the timeout operator as most recent operator.
     * \warning #include <rpp/operators/timeout.hpp>
     * 
     * \par Example
     * \snippet timeout.cpp timeout
	 *
     * \ingroup utility_operators
     * \see https://reactivex.io/documentation/operators/timeout.html
     */
    template<schedulers::constraint::scheduler TScheduler>
    auto timeout(schedulers::duration period, const TScheduler& scheduler = TScheduler{}) const & requires is_header_included<timeout_tag, TScheduler>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(timeout_impl<Type, TScheduler>{period, scheduler});
    }

    template<schedulers::constraint::scheduler TScheduler>
    auto timeout(schedulers::duration period, const TScheduler& scheduler = TScheduler{}) && requires is_header_included<timeout_tag, TScheduler>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(timeout_impl<Type, TScheduler>{period, scheduler});
    }
};
} // namespace rpp::details
