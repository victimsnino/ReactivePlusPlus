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

#include <rpp/defs.hpp>
#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/operators/details/utils.hpp>
#include <rpp/schedulers/current_thread.hpp>

#include <memory>
#include <tuple>

namespace rpp::operators::details
{
template<rpp::constraint::observer Observer, typename TSelector, rpp::constraint::decayed_type... Args>
class combine_latest_disposable final : public composite_disposable
{
public:
    explicit combine_latest_disposable(Observer&& observer, const TSelector& selector)
        : observer_with_mutex{std::move(observer)}
        , selector{selector}
    {
    }

    pointer_under_lock<Observer> get_observer_under_lock() { return pointer_under_lock{observer_with_mutex}; }

    rpp::utils::tuple<std::optional<Args>...>& get_values() { return values; }

    const TSelector& get_selector() const { return selector; }

    bool decrement_on_completed()
    {
        // just need atomicity, not guarding anything
        return m_on_completed_needed.fetch_sub(1) == 1;
    }

private:
    value_with_mutex<Observer>                observer_with_mutex{};
    rpp::utils::tuple<std::optional<Args>...> values{};
    RPP_NO_UNIQUE_ADDRESS TSelector           selector;

    std::atomic_size_t m_on_completed_needed{sizeof...(Args)};
};

template<size_t I, rpp::constraint::observer Observer, typename TSelector, rpp::constraint::decayed_type... Args>
struct combine_latest_observer_strategy
{
    std::shared_ptr<combine_latest_disposable<Observer, TSelector, Args...>> disposable{};

    void set_upstream(const rpp::disposable_wrapper& d) const
    {
        disposable->add(d);
    }

    bool is_disposed() const
    {
        return disposable->is_disposed();
    }

    template<typename T>
    void on_next(T&& v) const
    {
        // mutex need to be locked during changing of values, generating new values and sending of new values due to we can't update value while we are sending old one
        const auto observer = disposable->get_observer_under_lock();
        disposable->get_values().template get<I>().emplace(std::forward<T>(v));

        disposable->get_values().apply([this, &observer](const std::optional<Args>&... vals) {
            if ((vals.has_value() && ...))
                observer->on_next(disposable->get_selector()(vals.value()...));
        });
    }

    void on_error(const std::exception_ptr& err) const
    {
        disposable->dispose();
        disposable->get_observer_under_lock()->on_error(err);
    }

    void on_completed() const
    {
        if (disposable->decrement_on_completed())
        {
            disposable->dispose();
            disposable->get_observer_under_lock()->on_completed();
        }
    }
};

template<typename TSelector, rpp::constraint::observable... TObservables>
struct combine_latest_t
{
    RPP_NO_UNIQUE_ADDRESS rpp::utils::tuple<TObservables...> observables;
    RPP_NO_UNIQUE_ADDRESS TSelector                          selector;

    template<rpp::constraint::decayed_type T>
        requires std::invocable<TSelector, T, rpp::utils::extract_observable_type_t<TObservables>...>
    using ResultValue = std::invoke_result_t<TSelector, T, rpp::utils::extract_observable_type_t<TObservables>...>;

    template<rpp::constraint::observer Observer, typename... Strategies>
    void subscribe(Observer&& observer, const observable_chain_strategy<Strategies...>& observable_strategy) const
    {
        // Need to take ownership over current_thread in case of inner-observables also uses them
        auto drain_on_exit = rpp::schedulers::current_thread::own_queue_and_drain_finally_if_not_owned();
        observables.apply(&subscribe_impl<Observer, Strategies...>, std::forward<Observer>(observer), observable_strategy, selector);
    }

private:
    template<rpp::constraint::observer Observer, typename... Strategies>
    static void subscribe_impl(Observer&& observer, const observable_chain_strategy<Strategies...>& observable_strategy, const TSelector& selector, const TObservables&... observables)
    {
        using ExpectedValue = typename observable_chain_strategy<Strategies...>::ValueType;
        using Disposable    = combine_latest_disposable<Observer, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>;

        auto disposable = std::make_shared<Disposable>(std::forward<Observer>(observer), selector);
        disposable->get_observer_under_lock()->set_upstream(rpp::disposable_wrapper::from_weak(disposable));
        subscribe<std::decay_t<ExpectedValue>>(disposable, std::index_sequence_for<TObservables...>{}, observables...);

        observable_strategy.subscribe(rpp::observer<ExpectedValue, combine_latest_observer_strategy<0, std::decay_t<Observer>, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>>{std::move(disposable)});
    }

    template<typename ExpectedValue, rpp::constraint::observer Observer, size_t... I>
    static void subscribe(std::shared_ptr<combine_latest_disposable<Observer, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>> disposable, std::index_sequence<I...>, const TObservables&... observables)
    {
        (..., observables.subscribe(rpp::observer<rpp::utils::extract_observable_type_t<TObservables>, combine_latest_observer_strategy<I + 1, Observer, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>>{disposable}));
    }
};
}

namespace rpp::operators
{
/**
 * @brief Combines latest emissions from observables with emission from current observable when any observable sends new value via applying selector
 *
 * @marble combine_latest_custom_selector
   {
       source observable                                 : +------1    -2    --    -3    -|
       source other_observable                           : +-5-6-7-    --    -8    --    -|
       operator "combine_latest: x,y =>std::pair{x,y}"   : +------{1,5}-{2,7}-{2,8}-{3,8}-|
   }
 *
 * @details Actually this operator subscribes on all of theses observables and emits new combined value when any of them emits new emission (and each observable emit values at least one to be able to provide combined value)
 *
 * @par Performance notes:
 * - 1 heap allocation for disposable
 * - each value from any observable copied/moved to internal storage
 * - mutex acquired every time value obtained
 *
 * @param selector is applied to current emission of current observable and latests emissions from observables
 * @param observables are observables whose emissions would be combined with current observable
 * @warning #include <rpp/operators/combine_latest.hpp>
 *
 * @par Examples
 * @snippet combine_latest.cpp combine_latest custom selector
 *
 * @ingroup combining_operators
 * @see https://reactivex.io/documentation/operators/combinelatest.html
 */
template<typename TSelector, rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires (!rpp::constraint::observable<TSelector> && (!utils::is_not_template_callable<TSelector> || std::invocable<TSelector, rpp::utils::convertible_to_any, utils::extract_observable_type_t<TObservable>, utils::extract_observable_type_t<TObservables>...>))
auto combine_latest(TSelector&& selector, TObservable&& observable, TObservables&&... observables)
{
    return details::combine_latest_t<std::decay_t<TSelector>, std::decay_t<TObservable>, std::decay_t<TObservables>...>{
        rpp::utils::tuple{std::forward<TObservable>(observable), std::forward<TObservables>(observables)...},
        std::forward<TSelector>(selector)
    };
}

/**
 * @brief Combines latest emissions from observables with emission from current observable when any observable sends new value via making tuple
 *
 * @marble combine_latest
   {
       source observable                       : +------1    -2    --    -3    -|
       source other_observable                 : +-5-6-7-    --    -8    --    -|
       operator "combine_latest: make_tuple"   : +------{1,5}-{2,7}-{2,8}-{3,8}-|
   }
 *
 * @details Actually this operator subscribes on all of theses observables and emits new combined value when any of them emits new emission (and each observable emit values at least one to be able to provide combined value)
 *
 * @warning Selector is just packing values to tuple in this case
 *
 * @par Performance notes:
 * - 1 heap allocation for disposable
 * - each value from any observable copied/moved to internal storage
 * - mutex acquired every time value obtained
 *
 * @param observables are observables whose emissions would be combined when any observable sends new value
 * @warning #include <rpp/operators/combine_latest.hpp>
 *
 * @par Examples
 * @snippet combine_latest.cpp combine_latest
 *
 * @ingroup combining_operators
 * @see https://reactivex.io/documentation/operators/combinelatest.html
 */
template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
auto combine_latest(TObservable&& observable, TObservables&&... observables)
{
    return combine_latest(rpp::utils::pack_to_tuple{}, std::forward<TObservable>(observable), std::forward<TObservables>(observables)...);
}
} // namespace rpp::operators
