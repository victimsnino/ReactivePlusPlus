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
#include <rpp/operators/details/utils.hpp>
#include <rpp/defs.hpp>

#include <tuple>


namespace rpp::operators::details
{
template<rpp::constraint::observer Observer, typename TSelector, rpp::constraint::decayed_type... RestArgs>
    requires std::invocable<TSelector, rpp::utils::extract_observer_type_t<Observer>, RestArgs...>
struct with_latest_from_disposable final : public composite_disposable
{
    explicit with_latest_from_disposable(Observer&& observer, const TSelector& selector) 
        : observer{std::move(observer)}
        , selector{selector}
        {}

    Observer                                                observer;
    std::mutex                                              observer_mutex{};
    rpp::utils::tuple<utils::value_with_mutex<RestArgs>...> values{};
    TSelector                                               selector;
};

template<size_t I, rpp::constraint::observer Observer, typename TSelector, rpp::constraint::decayed_type... RestArgs>
struct with_latest_from_inner_observer_strategy
{
    std::shared_ptr<with_latest_from_disposable<Observer, TSelector, RestArgs...>> disposable{};

    void set_upstream(const rpp::disposable_wrapper& d) const
    {
        disposable->add(d.get_original());
    }

    bool is_disposed() const
    {
        return disposable->is_disposed();
    }

    template<typename T>
    void on_next(T&& v) const 
    {
        auto& [value, mutex] = disposable->values.template get<I>();
        std::scoped_lock lock{mutex};
        value.emplace(std::forward<T>(v));
    }

    void on_error(const std::exception_ptr& err) const
    {
        disposable->dispose();

        std::scoped_lock lock{disposable->observer_mutex};
        disposable->observer.on_error(err);
    }

    static constexpr rpp::utils::empty_function_t<> on_completed{};
};

template<rpp::constraint::observer Observer, typename TSelector, rpp::constraint::observable... TObservables>
    requires std::invocable<TSelector, rpp::utils::extract_observer_type_t<Observer>, rpp::utils::extract_observable_type_t<TObservables>...>
class with_latest_from_observer_strategy
{
    using Result = std::invoke_result_t<TSelector, rpp::utils::extract_observer_type_t<Observer>, rpp::utils::extract_observable_type_t<TObservables>...>;
    using Disposable = with_latest_from_disposable<Observer, TSelector, rpp::utils::extract_observable_type_t<TObservables>...>;
public:
    using DisposableStrategyToUseWithThis = rpp::details::none_disposable_strategy;

    template<size_t...I>
    with_latest_from_observer_strategy(Observer&& observer, const TSelector& selector, const TObservables&... observables, std::index_sequence<I...> = std::index_sequence_for<TObservables...>{})
        : m_disposable{std::make_shared<Disposable>(std::move(observer), selector)}
    {
        m_disposable->observer.set_upstream(rpp::disposable_wrapper::from_weak(m_disposable));
        (observables.subscribe(rpp::observer<rpp::utils::extract_observable_type_t<TObservables>, with_latest_from_inner_observer_strategy<I, Observer, TSelector, rpp::utils::extract_observable_type_t<TObservables>...>>{m_disposable}),...);
    }

    void set_upstream(const rpp::disposable_wrapper& d) const
    {
        m_disposable->add(d.get_original());
    }

    bool is_disposed() const
    {
        return m_disposable->is_disposed();
    }

    template<typename T>
    void on_next(T&& v) const 
    {
        auto result = m_disposable->values.apply([&](const utils::value_with_mutex<rpp::utils::extract_observable_type_t<TObservables>>&... vals) -> std::optional<Result>
        {
            auto lock = std::scoped_lock{vals.mutex...};

            if ((vals.value.has_value() && ...))
                return m_disposable->selector(rpp::utils::as_const(std::forward<T>(v)), rpp::utils::as_const(vals.value.value())...);
            return std::nullopt;
        });

        if (result.has_value())
        {
            std::scoped_lock lock{m_disposable->observer_mutex};
            m_disposable->observer.on_next(std::move(result).value());
        }
    }

    void on_error(const std::exception_ptr& err) const
    {
        m_disposable->dispose();

        std::scoped_lock lock{m_disposable->observer_mutex};
        m_disposable->observer.on_error(err);
    }

    void on_completed() const
    {
        m_disposable->dispose();

        std::scoped_lock lock{m_disposable->observer_mutex};
        m_disposable->observer.on_completed();
    }

private:
    std::shared_ptr<Disposable> m_disposable{};
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
        using InnerObservable = typename observable_chain_strategy<Strategies...>::ValueType;

        observable_strategy.subscribe(rpp::observer<InnerObservable, with_latest_from_observer_strategy<std::decay_t<Observer>, TSelector, TObservables...>>{std::forward<Observer>(observer), selector, observables...});
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
    return details::with_latest_from_t<std::decay_t<TSelector>, std::decay_t<TObservable>, std::decay_t<TObservables>...>{rpp::utils::tuple{std::forward<TObservable>(observable), std::forward<TObservables>(observables)...}, std::forward<TSelector>(selector)};
}
} // namespace rpp::operators
