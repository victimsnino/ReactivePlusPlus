//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/defs.hpp>
#include <rpp/sources/fwd.hpp>
#include <rpp/observables/observable.hpp>

namespace rpp::details
{
struct interval_schedulable
{
    rpp::schedulers::optional_duration operator()(const auto& observer, rpp::schedulers::duration period, size_t& counter) const
    {
         observer.on_next(counter++);
         return period;
    }
};

template<typename TScheduler>
struct interval_strategy
{
    using ValueType = size_t;

    RPP_NO_UNIQUE_ADDRESS TScheduler scheduler;
    rpp::schedulers::duration initial;
    rpp::schedulers::duration period;

    template<rpp::constraint::observer_of_type<ValueType> TObs>
    void subscribe(TObs&& observer) const
    {
        const auto worker = scheduler.create_worker();
        observer.set_upstream(worker.get_disposable());
        worker.schedule(initial, interval_schedulable{}, std::forward<TObs>(observer), period, size_t{});
    }
};
}

namespace rpp
{
template<schedulers::constraint::scheduler TScheduler>
using interval_observable = observable<size_t, details::interval_strategy<TScheduler>>;
}

namespace rpp::source
{

/**
 * @brief Creates rpp::observable that emits a sequential integer every specified time interval, on the specified scheduler.
 *
 * @marble interval_with_initial
   {
       operator "interval(20s, 10s)": +--1-2-3-5->
   }
 *
 * @param initial time before first emission
 * @param period period between emitted values
 * @param scheduler the scheduler to use for scheduling the items
 * 
 * @par Example:
 * @snippet interval.cpp interval period
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/interval.html
 */
template<schedulers::constraint::scheduler TScheduler>
auto interval(rpp::schedulers::duration initial, rpp::schedulers::duration period, TScheduler&& scheduler) {
    return interval_observable<std::decay_t<TScheduler>>{std::forward<TScheduler>(scheduler), initial, period};
}

/**
 * @brief Creates rpp::observable that emits a sequential integer every specified time interval, on the specified scheduler.
 *
 * @marble interval
   {
       operator "interval(10s)": +-1-2-3-5->
   }
 * @param period period between emitted values
 * @param scheduler the scheduler to use for scheduling the items
 * 
 * @par Example:
 * @snippet interval.cpp interval period
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/interval.html
 */
template<schedulers::constraint::scheduler TScheduler>
auto interval(rpp::schedulers::duration period, TScheduler&& scheduler) {
    return interval(period, period, std::forward<TScheduler>(scheduler));
}
}