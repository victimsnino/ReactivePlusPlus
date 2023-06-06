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

#include "rpp/observables/fwd.hpp"
#include <rpp/defs.hpp>
#include <rpp/observables/observable.hpp>
#include <rpp/operators/fwd.hpp>

namespace rpp::operators::details
{
template<rpp::schedulers::constraint::scheduler Scheduler, rpp::constraint::observable TObservable>
struct subscribe_on_strategy
{
    RPP_NO_UNIQUE_ADDRESS Scheduler scheduler;
    RPP_NO_UNIQUE_ADDRESS TObservable observable;

    using Type = rpp::utils::extract_observable_type_t<TObservable>;

    template<rpp::constraint::observer_strategy<Type> ObserverStrategy>
    void subscribe(observer<Type, ObserverStrategy>&& obs) const
    {
        auto worker = scheduler.create_worker();
        obs.set_upstream(worker.get_disposable());
        worker.schedule(
            [observable = observable](auto&& observer)
            {
                observable.subscribe(std::move(observer));
                return rpp::schedulers::optional_duration{};
            },
            std::move(obs));
    }
};

template<rpp::schedulers::constraint::scheduler Scheduler>
struct subscribe_on_t
{
    RPP_NO_UNIQUE_ADDRESS Scheduler scheduler;

    template<rpp::constraint::observable TObservable>
    auto operator()(TObservable&& observable) const
    {
        return rpp::observable<rpp::utils::extract_observable_type_t<TObservable>,
                               subscribe_on_strategy<Scheduler, std::decay_t<TObservable>>>{scheduler,
                                                                                            std::forward<TObservable>(observable)};
    }
};
}

namespace rpp::operators
{
template<rpp::schedulers::constraint::scheduler Scheduler>
auto subscribe_on(Scheduler&& scheduler)
{
    return details::subscribe_on_t<std::decay_t<Scheduler>>{std::forward<Scheduler>(scheduler)};
}
} // namespace rpp::operators