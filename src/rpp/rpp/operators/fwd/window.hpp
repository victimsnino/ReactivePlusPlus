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

#include <rpp/observables/constraints.hpp>
#include <rpp/observables/details/member_overload.hpp>

namespace rpp::details
{
struct window_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto window_impl(TObs&& obs, size_t window_size);

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs, schedulers::constraint::scheduler TScheduler>
auto window_with_time_impl(TObs&& obs, schedulers::duration period, const TScheduler& scheduler);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, window_tag>
{
   /**
    * \brief Subdivide original observable into sub-observables (windowed observables) and emit sub-observables of items instead of original items. Each new sub-observable emitted each `count`-nth emission (and previous completed)
    * 
    * \marble window
    {
        source observable    :  +-1-2-3-4-5-|

        operator "window(2)" : 
                            {   
                                .+1-2|
                                .....+3-4|
                                .........+5-|
                            }
    }
    *
    * \details Actually it is similar to `buffer` but it emits observable instead of container.
    *
    * \param window_size amount of items which every observable would have
    *
    * \return new specific_observable with the window operator as most recent operator.
    * \warning #include <rpp/operators/window.hpp>
    * 
    * \par Example
    * \snippet window.cpp window
    *   
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to keep internal state
    * - <b>OnNext</b>
    *    - Emits new window-observable if previous observable emitted requested amound of emisions
    *    - Emits emission via active window-observable
    *    - Completes window-observable if requested amound of emisions reached
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - Just forwards original on_completed 
    *
    * \ingroup transforming_operators
    * \see https://reactivex.io/documentation/operators/window.html
    */
    template<typename ...Args>
    auto window(size_t window_size) const & requires is_header_included<window_tag, Args...>
    {
        return window_impl<Type>(*static_cast<const SpecificObservable*>(this), window_size);
    }

    template<typename ...Args>
    auto window(size_t window_size) && requires is_header_included<window_tag, Args...>
    {
        return window_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)), window_size);
    }

    /**
    * \brief Subdivide original observable into sub-observables (windowed observables) and emit sub-observables of items instead of original items. Each new sub-observable emitted every `period` of time (and previous completed)
    * 
    * \marble window_with_time
    {
        source observable    :  +1-2-3----4-5|

        operator "window_with_time(3)" : 
                            {   
                                +1-2|
                                ...+-3-|
                                ......+---|
                                .........+4-5|
                            }
    }
    *
    * \details Actually it is similar to `buffer_with_time` but it emits observable instead of container.
    *
    * \param period is period of time when previous observable would be completed and new one emitted
    *
    * \return new specific_observable with the window_with_time operator as most recent operator.
    * \warning #include <rpp/operators/window.hpp>
    * 
    * \par Example
    * \snippet window.cpp window_with_time
    *   
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to keep internal state
    *    - Schedules schedulable to complete current observable and emit new one
    * - <b>OnNext</b>
    *    - Emits emission via active window-observable
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - Just forwards original on_completed 
    *
    * \ingroup transforming_operators
    * \see https://reactivex.io/documentation/operators/window.html
    */
    template<schedulers::constraint::scheduler TScheduler>
    auto window_with_time(schedulers::duration period, const TScheduler& scheduler) const & requires is_header_included<window_tag,TScheduler>
    {
        return window_with_time_impl<Type>(*static_cast<const SpecificObservable*>(this), period, scheduler);
    }

    template<schedulers::constraint::scheduler TScheduler>
    auto window_with_time(schedulers::duration period, const TScheduler& scheduler) && requires is_header_included<window_tag,TScheduler>
    {
        return window_with_time_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)), period, scheduler);
    }
};
} // namespace rpp::details
