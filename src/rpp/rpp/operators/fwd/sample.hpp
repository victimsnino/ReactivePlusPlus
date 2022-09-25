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

namespace rpp::details
{
struct sample_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct sample_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, sample_tag>
{
    /**
     * \brief 
     * 
     * \marble sample
        {
            
        }
     *
     * \return new specific_observable with the sample operator as most recent operator.
     * \warning #include <rpp/operators/sample.hpp>
     * 
     * \par Example
     * \snippet sample.cpp sample
	 *
     * \ingroup 
     * \see
     */
    template<schedulers::constraint::scheduler TScheduler = rpp::schedulers::immediate, typename ...Args>
    auto sample(schedulers::duration period, const TScheduler& scheduler = TScheduler{}) const & requires is_header_included<sample_tag, TScheduler, Args...>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(sample_impl<Type, TScheduler>{period, scheduler});
    }

    template<schedulers::constraint::scheduler TScheduler = rpp::schedulers::immediate, typename ...Args>
    auto sample(schedulers::duration period, const TScheduler& scheduler = TScheduler{}) && requires is_header_included<sample_tag, TScheduler, Args...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(sample_impl<Type, TScheduler>{period, scheduler});
    }
};
} // namespace rpp::details
