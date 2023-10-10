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
template<rpp::constraint::observer Observer, typename TSelector, rpp::constraint::decayed_type... RestArgs>
class with_latest_from_disposable final : public composite_disposable
{
public:
    explicit with_latest_from_disposable(Observer&& observer, const TSelector& selector)
        : observer_with_mutex{std::move(observer)}
        , selector{selector}
    {
    }

    pointer_under_lock<Observer> get_observer_under_lock() { return pointer_under_lock{observer_with_mutex}; }

    rpp::utils::tuple<value_with_mutex<std::optional<RestArgs>>...>& get_values() { return values; }

    const TSelector& get_selector() const { return selector; }

private:
    value_with_mutex<Observer>                                      observer_with_mutex{};
    rpp::utils::tuple<value_with_mutex<std::optional<RestArgs>>...> values{};
    RPP_NO_UNIQUE_ADDRESS TSelector                                 selector;
};

template<size_t I, rpp::constraint::observer Observer, typename TSelector, rpp::constraint::decayed_type... RestArgs>
struct with_latest_from_inner_observer_strategy
{
    std::shared_ptr<with_latest_from_disposable<Observer, TSelector, RestArgs...>> disposable{};

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
        auto& [value, mutex] = disposable->get_values().template get<I>();
        std::scoped_lock lock{mutex};
        value.emplace(std::forward<T>(v));
    }

    void on_error(const std::exception_ptr& err) const
    {
        disposable->dispose();
        disposable->get_observer_under_lock()->on_error(err);
    }

    static constexpr rpp::utils::empty_function_t<> on_completed{};
};

template<rpp::constraint::observer Observer, typename TSelector, typename OriginalValue, rpp::constraint::decayed_type... RestArgs>
    requires std::invocable<TSelector, OriginalValue, RestArgs...>
struct with_latest_from_observer_strategy
{
    using Disposable                      = with_latest_from_disposable<Observer, TSelector, RestArgs...>;
    using Result                          = std::invoke_result_t<TSelector, OriginalValue, RestArgs...>;
    using DisposableStrategyToUseWithThis = rpp::details::observers::none_disposable_strategy;

    std::shared_ptr<Disposable> disposable{};

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
        auto result = disposable->get_values().apply([this, &v](value_with_mutex<std::optional<RestArgs>>&... vals) -> std::optional<Result> {
            auto lock = std::scoped_lock{vals.mutex...};

            if ((vals.value.has_value() && ...))
                return disposable->get_selector()(rpp::utils::as_const(std::forward<T>(v)), rpp::utils::as_const(vals.value.value())...);
            return std::nullopt;
        });

        if (result.has_value())
            disposable->get_observer_under_lock()->on_next(std::move(result).value());
    }

    void on_error(const std::exception_ptr& err) const
    {
        disposable->dispose();
        disposable->get_observer_under_lock()->on_error(err);
    }

    void on_completed() const
    {
        disposable->dispose();
        disposable->get_observer_under_lock()->on_completed();
    }
};

template<typename TSelector, rpp::constraint::observable... TObservables>
struct with_latest_from_t
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
        using Disposable = with_latest_from_disposable<Observer, TSelector, rpp::utils::extract_observable_type_t<TObservables>...>;

        auto disposable = std::make_shared<Disposable>(std::forward<Observer>(observer), selector);
        disposable->get_observer_under_lock()->set_upstream(rpp::disposable_wrapper::from_weak(disposable));
        subscribe(disposable, std::index_sequence_for<TObservables...>{}, observables...);

        using ExpectedValue = typename observable_chain_strategy<Strategies...>::ValueType;

        observable_strategy.subscribe(rpp::observer<ExpectedValue, with_latest_from_observer_strategy<std::decay_t<Observer>, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>>{std::move(disposable)});
    }

    template<rpp::constraint::observer Observer, size_t... I>
    static void subscribe(std::shared_ptr<with_latest_from_disposable<Observer, TSelector, rpp::utils::extract_observable_type_t<TObservables>...>> disposable, std::index_sequence<I...>, const TObservables&... observables)
    {
        (..., observables.subscribe(rpp::observer<rpp::utils::extract_observable_type_t<TObservables>, with_latest_from_inner_observer_strategy<I, Observer, TSelector, rpp::utils::extract_observable_type_t<TObservables>...>>{disposable}));
    }
};
}

namespace rpp::operators
{
/**
 * @brief Combines latest emissions from observables with emission from current observable when it sends new value via applying selector
 *
 * @marble with_latest_from_custom_selector
   {
       source observable                                 : +------1    -2    -3    -|
       source other_observable                           : +-5-6-7-    --    8-    -|
       operator "with_latest_from: x,y =>std::pair{x,y}" : +------{1,5}-{2,7}-{3,8}-|
   }
 *
 * @details Actually this operator just keeps last values from all other observables and combines them together with each new emission from original observable
 *
 * @par Performance notes:
 * - 1 heap allocation for disposable
 * - each value from "others" copied/moved to internal storage
 * - mutex acquired every time value obtained
 *
 * @param selector is applied to current emission of current observable and latests emissions from observables
 * @param observables are observables whose emissions would be combined when current observable sends new value
 * @warning #include <rpp/operators/with_latest_from.hpp>
 *
 * @par Examples
 * @snippet with_latest_from.cpp with_latest_from custom selector
 *
 * @ingroup combining_operators
 * @see https://reactivex.io/documentation/operators/combinelatest.html
 */
template<typename TSelector, rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires (!rpp::constraint::observable<TSelector> && (!utils::is_not_template_callable<TSelector> || std::invocable<TSelector, rpp::utils::convertible_to_any, utils::extract_observable_type_t<TObservable>, utils::extract_observable_type_t<TObservables>...>))
auto with_latest_from(TSelector&& selector, TObservable&& observable, TObservables&&... observables)
{
    return details::with_latest_from_t<std::decay_t<TSelector>, std::decay_t<TObservable>, std::decay_t<TObservables>...>{
        rpp::utils::tuple{std::forward<TObservable>(observable), std::forward<TObservables>(observables)...},
        std::forward<TSelector>(selector)
    };
}

/**
 * @brief Combines latest emissions from observables with emission from current observable when it sends new value via making tuple
 *
 * @marble with_latest_from
   {
       source observable                       : +------1    -2    -3    -|
       source other_observable                 : +-5-6-7-    --    8-    -|
       operator "with_latest_from: make_tuple" : +------{1,5}-{2,7}-{3,8}-|
   }
 *
 * @warning Selector is just packing values to tuple in this case
 *
 * @par Performance notes:
 * - 1 heap allocation for disposable
 * - each value from "others" copied/moved to internal storage
 * - mutex acquired every time value obtained
 *
 * @param observables are observables whose emissions would be combined when current observable sends new value
 * @warning #include <rpp/operators/with_latest_from.hpp>
 *
 * @par Examples
 * @snippet with_latest_from.cpp with_latest_from
 *
 * @ingroup combining_operators
 * @see https://reactivex.io/documentation/operators/combinelatest.html
 */
template<rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
auto with_latest_from(TObservable&& observable, TObservables&&... observables)
{
    return with_latest_from(rpp::utils::pack_to_tuple{}, std::forward<TObservable>(observable), std::forward<TObservables>(observables)...);
}
} // namespace rpp::operators
