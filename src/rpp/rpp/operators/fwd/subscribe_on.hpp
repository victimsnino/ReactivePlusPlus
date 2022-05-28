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
#include <rpp/observables/constraints.hpp>
#include <rpp/schedulers/constraints.hpp>


namespace rpp::details
{
struct subscribe_on_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs, schedulers::constraint::scheduler TScheduler>
auto subscribe_on_impl(TObs&& obs, TScheduler&& scheduler);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, subscribe_on_tag>
{
    /**
    * \brief OnSubscribe function for this observable will be scheduled via provided scheduler
    * 
    * \param scheduler is scheduler used for scheduling of OnSubscribe
    * \return new specific_observable with the subscribe_on operator as most recent operator.
    * \warning #include <rpp/operators/subscribe_on.hpp>
    * 
    * \par Example:
    * \snippet subscribe_on.cpp subscribe_on
    * 
    * \ingroup utility_operators
    * \see https://reactivex.io/documentation/operators/subscribeon.html
    */
    template<schedulers::constraint::scheduler TScheduler>
    auto subscribe_on(const TScheduler& scheduler) const & requires is_header_included<subscribe_on_tag, TScheduler>
    {
        return subscribe_on_impl<Type>(*static_cast<const SpecificObservable*>(this), scheduler);
    }
    
    template<schedulers::constraint::scheduler TScheduler>
    auto subscribe_on(const TScheduler& scheduler) && requires is_header_included<subscribe_on_tag, TScheduler>
    {
        return subscribe_on_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)), scheduler);
    }
};
} // namespace rpp::details
