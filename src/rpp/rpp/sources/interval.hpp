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

#include <rpp/sources/fwd.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/schedulers/fwd.hpp>
#include <rpp/schedulers/immediate_scheduler.hpp>

#include <type_traits>


IMPLEMENTATION_FILE(interval_tag);

namespace rpp::observable
{
    /**
     * \brief Creates rpp::specific_observable which emits sequence of size_t every provided time interval.
     *
     * \marble interval
       {
           operator "interval: 200": +--0--1--2--3--4--5--6--
       }
     *
     * \warn First emission also scheduled and delayed with same interval
     *
     * \param period period which would be used to delay emissions between each other
     * \param scheduler used for scheduling this periodic emissions
     * \return rpp::specific_observable which emits values with provided time_interval
     *
     * \par Examples:
     * \snippet interval.cpp interval
     *
     * \ingroup creational_operators
     * \see https://reactivex.io/documentation/operators/interval.html
     */
    template<schedulers::constraint::scheduler TScheduler /*= schedulers::immediate*/>
    auto interval(schedulers::duration period, const TScheduler& scheduler /* = TScheduler{} */) requires rpp::details::is_header_included<rpp::details::interval_tag, TScheduler>
    {
        return interval(period, period, scheduler);
    }

    /**
     * \brief Creates rpp::specific_observable which emits sequence of size_t every provided time interval with first emission after provided delay
     *
     * \marble interval_init
       {
           operator "interval: 100, 200": +-0---1---2---3---4---5---6--
       }
     *
     * \param first_delay period which would be used to delay first emission
     * \param period period which would be used to delay emissions between each other
     * \param scheduler used for scheduling this periodic emissions
     * \return rpp::specific_observable which emits values with provided time_interval
     *
     * \par Examples:
     * \snippet interval.cpp interval_with_first
     *
     * \ingroup creational_operators
     * \see https://reactivex.io/documentation/operators/interval.html
     */
    template<schedulers::constraint::scheduler TScheduler /*= schedulers::immediate*/>
    auto interval(schedulers::duration first_delay, schedulers::duration period, const TScheduler& scheduler /* = TScheduler{} */) requires rpp::details::is_header_included<rpp::details::interval_tag, TScheduler>
    {
        return source::create<size_t>([first_delay, period, scheduler](auto&& subscriber)
        {
            auto worker = scheduler.create_worker(subscriber.get_subscription());
            worker.schedule(first_delay, [counter = size_t{}, subscriber = std::forward<decltype(subscriber)>(subscriber), period]() mutable-> schedulers::optional_duration
            {
                subscriber.on_next(counter++);
                return period;
            });
        });
    }
} // namespace rpp::observable