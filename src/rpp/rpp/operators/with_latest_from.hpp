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
#include <rpp/disposables/composite_disposable.hpp>

#include <rpp/schedulers/current_thread.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/defs.hpp>

#include <tuple>


namespace rpp::operators::details
{
template<rpp::constraint::observer Observer, rpp::constraint::decayed_type... RestArgs>
class with_latest_from_disposable final : public composite_disposable
{
public:
private:
    Observer                                      m_observer;
    rpp::utils::tuple<std::optional<RestArgs>...> m_values{};
};

template<rpp::constraint::observer Observer, typename TSelector, rpp::constraint::observable... TObservables>
    requires std::invocable<TSelector, rpp::utils::extract_observer_type_t<Observer>, utils::extract_observable_type_t<TObservables>...>
struct with_latest_from_observer_strategy
{
    using DisposableStrategyToUseWithThis = rpp::details::none_disposable_strategy;


};

template<typename TSelector, rpp::constraint::observable... TObservables>
struct with_latest_from_t
{
    RPP_NO_UNIQUE_ADDRESS rpp::utils::tuple<TObservables...> observables{};

    template<rpp::constraint::decayed_type T>
        requires std::invocable<TSelector, T, utils::extract_observable_type_t<TObservables>...>
    using ResultValue = std::invoke_result_t<TSelector, T, utils::extract_observable_type_t<TObservables>...>;

    template<rpp::constraint::observer Observer, typename... Strategies>
    void subscribe(Observer&& observer, const observable_chain_strategy<Strategies...>& observable_strategy) const
    {
        // Need to take ownership over current_thread in case of inner-observables also uses them
        auto drain_on_exit = rpp::schedulers::current_thread::own_queue_and_drain_finally_if_not_owned();

        using InnerObservable = typename observable_chain_strategy<Strategies...>::ValueType;

        observable_strategy.subscribe(rpp::observer<InnerObservable, with_latest_from_observer_strategy<std::decay_t<Observer>, TSelector, TObservables...>>{std::forward<Observer>(observer)});
    }

};
}

namespace rpp::operators
{
template<typename TSelector, rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires(!utils::is_not_template_callable<TSelector> ||
             std::invocable<TSelector, rpp::utils::convertible_to_any, utils::extract_observable_type_t<TObservable>, utils::extract_observable_type_t<TObservables>...>)
auto with_latest_from(TSelector&& selector, TObservable&& observable, TObservables&&... observables)
{
    return details::with_latest_from_t<std::decay_t<TSelector>, std::decay_t<TObservable>, std::decay_t<TObservables>...>{std::forward<TSelector>(selector), rpp::utils::tuple{std::forward<TObservable>(observable), std::forward<TObservables>(observables)...}};
}
} // namespace rpp::operators
