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

#include <rpp/defs.hpp>
#include <rpp/observables/observable.hpp>
#include <rpp/operators/fwd.hpp>

namespace rpp::operators::details
{
template<typename TObservableChainStrategy>
struct subscribe_on_schedulable
{
    RPP_NO_UNIQUE_ADDRESS TObservableChainStrategy observable;

    using Type = typename TObservableChainStrategy::ValueType;

    template<rpp::constraint::observer_strategy<Type> ObserverStrategy>
    rpp::schedulers::optional_duration operator()(observer<Type, ObserverStrategy>& observer) const
    {
        observable.subscribe(std::move(observer));
        return rpp::schedulers::optional_duration{};
    }
};


template<rpp::schedulers::constraint::scheduler Scheduler>
struct subscribe_on_t
{
    template<rpp::constraint::decayed_type T>
    using ResultValue = T;

    RPP_NO_UNIQUE_ADDRESS Scheduler scheduler;

    template<rpp::constraint::observer Observer, typename... Strategies>
    void subscribe(Observer&& observer, const observable_chain_strategy<Strategies...>& observable_strategy) const
    {
        const auto worker = scheduler.create_worker();
        if (auto d = worker.get_disposable(); !d.is_disposed())
            observer.set_upstream(rpp::disposable_wrapper::with_can_be_replaced_on_set_upstream(std::move(d)));
        worker.schedule(subscribe_on_schedulable<observable_chain_strategy<Strategies...>>{observable_strategy}, std::forward<Observer>(observer));
    }
};
}

namespace rpp::operators
{
/**
 * @brief OnSubscribe function for this observable will be scheduled via provided scheduler
 *
 * @details Actually this operator just schedules subscription on original observable to provided scheduler
 *
 * @param scheduler is scheduler used for scheduling of OnSubscribe
 * @warning #include <rpp/operators/subscribe_on.hpp>
 *
 * @par Example:
 * @snippet subscribe_on.cpp subscribe_on
 *
 * @ingroup utility_operators
 * @see https://reactivex.io/documentation/operators/subscribeon.html
 */
template<rpp::schedulers::constraint::scheduler Scheduler>
auto subscribe_on(Scheduler&& scheduler)
{
    return details::subscribe_on_t<std::decay_t<Scheduler>>{std::forward<Scheduler>(scheduler)};
}
} // namespace rpp::operators
