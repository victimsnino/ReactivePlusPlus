#pragma once

#include <rpp/sources/fwd.hpp>

#include <rpp/operators/take.hpp>
#include <rpp/sources/interval.hpp>

namespace rpp::source
{
/**
 * @brief Creates rpp::observable that emits an integer after a given delay, on the specified scheduler.
 *
 * @marble timer
   {
       operator "timer(1s)": +--0|
   }
 *
 * @param when duration from now when the value is emitted 
 * @param scheduler the scheduler to use for scheduling the items
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/timer.html
 */
template<schedulers::constraint::scheduler TScheduler>
auto timer(rpp::schedulers::duration when, TScheduler&& scheduler)
{
    return interval(when, std::forward<TScheduler>(scheduler)) | operators::take(1);
}

/**
 * @brief Same as rpp::source::timer but using a time_point as delay instead of a duration.
 *
 * @param when time point when the value is emitted 
 * @param scheduler the scheduler to use for scheduling the items
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/timer.html
 */
template<schedulers::constraint::scheduler TScheduler>
auto timer(rpp::schedulers::time_point when, TScheduler&& scheduler)
{
    return timer(when - rpp::schedulers::clock_type::now(), std::forward<TScheduler>(scheduler));
}
}