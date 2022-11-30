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

#include <rpp/schedulers/constraints.hpp>               // schedulers::constraint::scheduler_not_trampoline
#include <rpp/observables/details/member_overload.hpp>  // member_overload

namespace rpp::details
{
struct delay_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, schedulers::constraint::scheduler_not_trampoline TScheduler>
struct delay_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, delay_tag>
{

    /**
     * \brief Shift the emissions from an Observable forward in time by a particular amount.
     * \details The delay operator modifies its source Observable by pausing for a particular increment of time (that you specify) before emitting each of the source Observable’s items. This has the effect of shifting the entire sequence of items emitted by the Observable forward in time by that specified increment.
     *
     * \marble delay
       {
           source observable        : +-1-2-3-|
           operator "delay: --"     : +---1-2-3-|
       }
     *
     * \param delay_duration is the delay duration for emitting items. Delay duration should be able to cast to rpp::schedulers::duration.
     * \param scheduler provides the threading model for delay. e.g. With a new thread scheduler, the observer sees the values in a new thread after a delay duration to the subscription.
     * \return new specific_observable with the delay operator as most recent operator.
     * \warning #include <rpp/operators/delay.hpp>
     *
     * \par Examples
     * \snippet delay.cpp delay
     *
     * \ingroup utility_operators
     * \see https://reactivex.io/documentation/operators/delay.html
     */
    template<schedulers::constraint::scheduler_not_trampoline TScheduler>
    auto delay(auto&& delay_duration,
               TScheduler&& scheduler) const& requires is_header_included<delay_tag, TScheduler>
    {
        return cast_this()->template lift<Type>(
            delay_impl<Type, std::decay_t<TScheduler>>{std::forward<TScheduler>(scheduler),
                                                       std::chrono::duration_cast<rpp::schedulers::duration>(delay_duration)});
    }

    template<schedulers::constraint::scheduler_not_trampoline TScheduler>
    auto delay(auto&& delay_duration,
               TScheduler&& scheduler) && requires is_header_included<delay_tag, TScheduler>
    {
        return move_this().template lift<Type>(
            delay_impl<Type, std::decay_t<TScheduler>>{std::forward<TScheduler>(scheduler),
                                                       std::chrono::duration_cast<rpp::schedulers::duration>(delay_duration)});
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
