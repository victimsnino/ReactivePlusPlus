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
#include <rpp/observers/base_observer.hpp>
#include <rpp/observables/fwd.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/utils/functors.hpp>
#include <utility>

namespace rpp::operators
{
template<rpp::constraint::decayed_type Type, rpp::constraint::observer_strategy<Type> ObserverStrategy>
class subscribe<base_observer<Type, ObserverStrategy>>
{
public:
    /**
     * @brief Subscribes passed observer to emissions from this observable.
     *
     * @warning Observer must be moved in to subscribe method. (Not recommended) If you need to copy observer, convert
     * it to dynamic_observer
     */
    explicit subscribe(base_observer<Type, ObserverStrategy>&& observer)
    : m_observer{std::move(observer)} {}

    /**
     * @brief Subscribe passed observer to emissions from this observable.
     * @details Special overloading for dynamic observer to enable copy of observer
     */
    explicit subscribe(const dynamic_observer<Type>& observer)
        requires std::same_as<ObserverStrategy, details::observer::dynamic_strategy<Type>>
        : m_observer{observer}
    {
    }

    template<rpp::constraint::observable_strategy<Type> Strategy>
    void operator()(const rpp::base_observable<Type, Strategy>& observalbe) && {
        if (!m_observer.is_disposed())
            observalbe.subscribe(std::move(m_observer));
    }

private:
    base_observer<Type, ObserverStrategy> m_observer;
};

template<rpp::constraint::decayed_type Type, rpp::constraint::observer_strategy<Type> ObserverStrategy>
class subscribe<rpp::disposable_wrapper, base_observer<Type, ObserverStrategy>>
{
public:
    /**
     * @brief Subscribe passed observer to emissions from this observable.
     * @details This overloading attaches passed disposable to observer and return it to provide ability to dispose observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     *
     * @param d is disposable to be attached to observer. If disposable is nullptr or disposed -> no any subscription happens
     * @return disposable_wrapper disposable to be able to dispose observer when it needed
     */
    explicit subscribe(rpp::disposable_wrapper d, base_observer<Type, ObserverStrategy>&& observer)
        : m_disposable{std::move(d)}
        , m_observer{std::move(observer)} {}

    /**
     * @brief Subscribe passed observer to emissions from this observable.
     * @details This overloading attaches passed disposable to observer and return it to provide ability to dispose observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     * @details Special overloading for dynamic observer to enable copy of observer
     *
     * @param d is disposable to be attached to observer. If disposable is nullptr or disposed -> no any subscription happens
     * @return disposable_wrapper disposable to be able to dispose observer when it needed
     */
    explicit subscribe(rpp::disposable_wrapper d, const dynamic_observer<Type>& observer)
        requires std::same_as<ObserverStrategy, details::observer::dynamic_strategy<Type>>
        : m_disposable{std::move(d)}
        , m_observer{observer}
    {
    }

    template<rpp::constraint::observable_strategy<Type> Strategy>
    rpp::disposable_wrapper operator()(const rpp::base_observable<Type, Strategy>& observalbe) && {
        if (!m_disposable.is_disposed() && !m_observer.is_disposed())
            observalbe.subscribe(base_observer<Type, base_observer<Type, ObserverStrategy>>{m_disposable, std::move(m_observer)});
        return m_disposable;
    }

private:
    rpp::disposable_wrapper               m_disposable;
    base_observer<Type, ObserverStrategy> m_observer;
};

template<typename OnNext, std::invocable<const std::exception_ptr&> OnError, std::invocable<> OnCompleted>
class subscribe<OnNext, OnError, OnCompleted>
{
public:
    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     */
    template<rpp::constraint::decayed_same_as<OnNext> TOnNext, rpp::constraint::decayed_same_as<OnError> TOnError = OnError, rpp::constraint::decayed_same_as<OnCompleted> TOnCompleted = OnCompleted>
        requires (!constraint::decayed_same_as<TOnNext, subscribe<OnNext, OnError, OnCompleted>>)
    explicit subscribe(TOnNext&& on_next, TOnError&& on_error = {}, TOnCompleted&& on_completed = {}) // NOLINT
        : m_on_next{std::forward<TOnNext>(on_next)}
        , m_on_error{std::forward<TOnError>(on_error)}
        , m_on_completed{std::forward<TOnCompleted>(on_completed)}
    {
    }


    template<rpp::constraint::decayed_type Type, rpp::constraint::observable_strategy<Type> Strategy>
        requires std::invocable<OnNext, Type>
    void operator()(const rpp::base_observable<Type, Strategy>& observalbe) &&
    {
        observalbe.subscribe(make_lambda_observer<Type>(std::move(m_on_next), std::move(m_on_error), std::move(m_on_completed)));
    }

    template<rpp::constraint::decayed_type Type, rpp::constraint::observable_strategy<Type> Strategy>
        requires std::invocable<OnNext, Type>
    void operator()(const rpp::base_observable<Type, Strategy>& observalbe) const &
    {
        observalbe.subscribe(make_lambda_observer<Type>(m_on_next, m_on_error, m_on_completed));
    }

private:
    RPP_NO_UNIQUE_ADDRESS OnNext m_on_next;
    RPP_NO_UNIQUE_ADDRESS OnError m_on_error;
    RPP_NO_UNIQUE_ADDRESS OnCompleted m_on_completed;
};

template<typename OnNext>
class subscribe<OnNext> : public subscribe<OnNext, rpp::utils::rethrow_error_t, rpp::utils::empty_function_t<>>
{
public:
    using subscribe<OnNext, rpp::utils::rethrow_error_t, rpp::utils::empty_function_t<>>::subscribe;
};

template<typename OnNext, std::invocable<const std::exception_ptr&> OnError>
class subscribe<OnNext, OnError> : public subscribe<OnNext, OnError, rpp::utils::empty_function_t<>>
{
public:
    using subscribe<OnNext, OnError, rpp::utils::empty_function_t<>>::subscribe;
};

template<typename OnNext, std::invocable<const std::exception_ptr&> OnError, std::invocable<> OnCompleted>
class subscribe<rpp::disposable_wrapper, OnNext, OnError, OnCompleted>
{
public:
    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     */
    template<rpp::constraint::decayed_same_as<OnNext> TOnNext, rpp::constraint::decayed_same_as<OnError> TOnError = OnError, rpp::constraint::decayed_same_as<OnCompleted> TOnCompleted = OnCompleted>
    explicit subscribe(rpp::disposable_wrapper d, TOnNext&& on_next, TOnError&& on_error = {}, TOnCompleted&& on_completed = {})
        : m_disposable{std::move(d)}
        , m_on_next{std::forward<TOnNext>(on_next)}
        , m_on_error{std::forward<TOnError>(on_error)}
        , m_on_completed{std::forward<TOnCompleted>(on_completed)}
    {
    }


    template<rpp::constraint::decayed_type Type, rpp::constraint::observable_strategy<Type> Strategy>
        requires std::invocable<OnNext, Type>
    rpp::disposable_wrapper operator()(const rpp::base_observable<Type, Strategy>& observalbe) &&
    {
        if (!m_disposable.is_disposed())
            observalbe.subscribe(make_lambda_observer<Type>(m_disposable, std::move(m_on_next), std::move(m_on_error), std::move(m_on_completed)));
        return std::move(m_disposable);
    }

    template<rpp::constraint::decayed_type Type, rpp::constraint::observable_strategy<Type> Strategy>
        requires std::invocable<OnNext, Type>
    rpp::disposable_wrapper operator()(const rpp::base_observable<Type, Strategy>& observalbe) const &
    {
        if (!m_disposable.is_disposed())
            observalbe.subscribe(make_lambda_observer<Type>(m_disposable, m_on_next, m_on_error, m_on_completed));
        return m_disposable;
    }

private:
    rpp::disposable_wrapper m_disposable;
    RPP_NO_UNIQUE_ADDRESS OnNext                    m_on_next;
    RPP_NO_UNIQUE_ADDRESS OnError                   m_on_error;
    RPP_NO_UNIQUE_ADDRESS OnCompleted               m_on_completed;
};

template<typename OnNext>
class subscribe<rpp::disposable_wrapper, OnNext> : public subscribe<rpp::disposable_wrapper, OnNext, rpp::utils::rethrow_error_t, rpp::utils::empty_function_t<>>
{
public:
    using subscribe<rpp::disposable_wrapper, OnNext, rpp::utils::rethrow_error_t, rpp::utils::empty_function_t<>>::subscribe;
};

template<typename OnNext, std::invocable<const std::exception_ptr&> OnError>
class subscribe<rpp::disposable_wrapper, OnNext, OnError> : public subscribe<rpp::disposable_wrapper, OnNext, OnError, rpp::utils::empty_function_t<>>
{
public:
    using subscribe<rpp::disposable_wrapper, OnNext, OnError, rpp::utils::empty_function_t<>>::subscribe;
};

template<typename ...Args>
subscribe(const Args&...) -> subscribe<Args...>;
}