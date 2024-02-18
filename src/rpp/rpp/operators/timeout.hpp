//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/operators/fwd.hpp>

#include <rpp/operators/details/strategy.hpp>

namespace rpp::operators::details
{
    template<rpp::constraint::observer TObserver, rpp::constraint::observable TFallbackObservable, rpp::schedulers::constraint::scheduler TScheduler>
    struct timeout_observer_strategy
    {
    };

    template<rpp::constraint::observable TFallbackObservable, rpp::schedulers::constraint::scheduler TScheduler>
    struct timeout_t : public lift_operator<timeout_t<TFallbackObservable, TScheduler>, rpp::schedulers::duration, TFallbackObservable, TScheduler>
    {
        template<rpp::constraint::decayed_type T>
        struct operator_traits
        {
            using result_type = T;

            template<rpp::constraint::observer_of_type<result_type> TObserver>
            using observer_strategy = timeout_observer_strategy<TObserver, TFallbackObservable, TScheduler>;
        };

        template<rpp::details::observables::constraint::disposable_strategy Prev>
        using updated_disposable_strategy = typename Prev::template add<rpp::schedulers::utils::get_worker_t<TScheduler>::is_none_disposable ? 0 : 1>;
    };
} // namespace rpp::operators::details

namespace rpp::operators
{
    template<rpp::constraint::observable TFallbackObservable, rpp::schedulers::constraint::scheduler TScheduler /* = rpp::schedulers::immediate*/>
    auto timeout(rpp::schedulers::duration period, TFallbackObservable&& fallback_observable, const TScheduler& scheduler /* = {}*/)
    {
        return details::timeout_t<std::decay_t<TFallbackObservable>, TScheduler>{period, std::forward<TFallbackObservable>(fallback_observable), scheduler};
    }
} // namespace rpp::operators