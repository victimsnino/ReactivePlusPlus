//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//                             TC Wang 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/details/member_overload.hpp>
#include <rpp/schedulers/constraints.hpp>

namespace rpp::details
{
struct observe_on_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct observe_on_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, observe_on_tag>
{
    /**
    * \brief Emit emissions of observable starting from this point via provided scheduler
    *
    * \details Actually this operator just schedules emissions via provided scheduler. So, actually it is delay(0) operator
    *
    * \param scheduler is scheduler used for scheduling of OnNext
    * \return new specific_observable with the observe_on operator as most recent operator.
    * \warning #include <rpp/operators/observe_on.hpp>
    *
    * \par Example:
    * \snippet observe_on.cpp observe_on
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to store internal state
    * - <b>OnNext</b>
    *    - Move emission to queue and schedule action to drain queue (if not yet)
    * - <b>OnError</b>
    *    - Just forwards original on_error via scheduling
    * - <b>OnCompleted</b>
    *    - Just forwards original on_completed via scheduling
    *
    * \ingroup utility_operators
    * \see https://reactivex.io/documentation/operators/observeon.html
    */
    template<schedulers::constraint::scheduler TScheduler>
    auto observe_on(TScheduler&& scheduler) const& requires is_header_included<observe_on_tag, TScheduler>
    {
        return cast_this()->delay(schedulers::duration{0}, std::forward<TScheduler>(scheduler));
    }

    template<schedulers::constraint::scheduler TScheduler>
    auto observe_on(TScheduler&& scheduler) && requires is_header_included<observe_on_tag, TScheduler>
    {
        return move_this().delay(schedulers::duration{0}, std::forward<TScheduler>(scheduler));
    }

private:
    const SpecificObservable* cast_this() const
    {
        return static_cast<const SpecificObservable*>(this);
    }

    SpecificObservable&& move_this()
    {
        return std::move(*static_cast<SpecificObservable*>(this));
    }
};
} // namespace rpp::details
