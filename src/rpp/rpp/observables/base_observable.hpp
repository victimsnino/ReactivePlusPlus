//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/fwd.hpp>
#include <rpp/observers/dynamic_observer.hpp>
#include <rpp/observers/lambda_observer.hpp>

#include <rpp/disposables/base_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/operators/subscribe.hpp>

#include <rpp/defs.hpp>


namespace rpp
{
/**
 * @brief Base class for any observable used in RPP. It handles core callbacks of observable.
 * @details Observable provides ony core function: subscribe - it accepts observer (or any way to construct it) and then invokes underlying Strategy to emit emissions somehow.
 * @warning Actually observable "doesn't emit nothing", it only invokes Strategy! Strategy COULD emit emissions immediately OR place observer to some place to obtain emissions later (for example subjects)
 *
 * @tparam Type of value this observable would provide
 * @tparam Strategy used to provide logic over observable's callbacks
 *
 * @ingroup observables
 */
template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class base_observable final
{
public:
    template<typename ...Args>
        requires (!constraint::variadic_decayed_same_as<base_observable<Type, Strategy>, Args...> && constraint::is_constructible_from<Strategy, Args&&...>)
    explicit base_observable(Args&& ...args)
        : m_strategy{std::forward<Args>(args)...} {}

    base_observable(const base_observable&)     = default;
    base_observable(base_observable&&) noexcept = default;

    /**
     * @brief Subscribes passed observer to emissions from this observable.
     *
     * @warning Observer must be moved in to subscribe method. (Not recommended) If you need to copy observer, convert it to dynamic_observer
     */
    template<constraint::observer_strategy<Type> ObserverStrategy>
    void subscribe(base_observer<Type, ObserverStrategy>&& observer) const
    {
        if (!observer.is_disposed())
            m_strategy.subscribe(std::move(observer));
    }

    /**
     * @brief Subscribe passed observer to emissions from this observable.
     * @details Special overloading for dynamic observer to enable copy of observer
     */
    void subscribe(dynamic_observer<Type> observer) const
    {
        subscribe<details::observer::dynamic_strategy<Type>>(std::move(observer));
    }

    /**
     * @brief Subscribe passed observer to emissions from this observable.
     * @details This overloading attaches passed disposable to observer and return it to provide ability to dispose observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     *
     * @param d is disposable to be attached to observer. If disposable is nullptr or disposed -> no any subscription happens
     * @return disposable_wrapper is disposable to be able to dispose observer when it needed
     */
    template<constraint::observer_strategy<Type> ObserverStrategy>
    disposable_wrapper subscribe(const disposable_wrapper& d, base_observer<Type, ObserverStrategy>&& observer) const
    {   if (!d.is_disposed())
            m_strategy.subscribe(base_observer<Type, rpp::details::with_disposable<base_observer<Type, ObserverStrategy>>>{d, std::move(observer)});
        return disposable_wrapper{d};
    }

    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     */
    template<std::invocable<Type> OnNext,
             std::invocable<const std::exception_ptr&> OnError = rpp::utils::rethrow_error_t,
             std::invocable<> OnCompleted = rpp::utils::empty_function_t<>>
    void subscribe(OnNext&&      on_next,
                   OnError&&     on_error = {},
                   OnCompleted&& on_completed = {}) const
    {
        subscribe(make_lambda_observer<Type>(std::forward<OnNext>(on_next),
                                             std::forward<OnError>(on_error),
                                             std::forward<OnCompleted>(on_completed)));
    }


    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     * @details This overloading attaches passed disposable to observer and return it to provide ability to dispose observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     *
     * @param d is disposable to be attached to observer. If disposable is nullptr or disposed -> no any subscription happens
     * @return disposable_wrapper is disposable to be able to dispose observer when it needed
     */
    template<std::invocable<Type>                      OnNext,
             std::invocable<const std::exception_ptr&> OnError     = rpp::utils::rethrow_error_t,
             std::invocable<>                          OnCompleted = rpp::utils::empty_function_t<>>
    disposable_wrapper subscribe(const disposable_wrapper& d, OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const
    {
        if (!d.is_disposed())
            m_strategy.subscribe(make_lambda_observer<Type>(d,
                                                std::forward<OnNext>(on_next),
                                                std::forward<OnError>(on_error),
                                                std::forward<OnCompleted>(on_completed)));
        return d;
    }

    template<constraint::operators<const base_observable<Type, Strategy>&> Op>
    auto operator|(Op&& op) const &
    {
        return std::forward<Op>(op)(*this);
    }

    template<constraint::operators<base_observable<Type, Strategy>&&> Op>
    auto operator|(Op&& op) &&
    {
        return std::forward<Op>(op)(std::move(*this));
    }

    template<typename...Args>
    auto operator|(const rpp::operators::subscribe<Args...>& op) const
    {
        return op(*this);
    }

    template<typename...Args>
    auto operator|(rpp::operators::subscribe<Args...>&& op) const
    {
        return std::move(op)(*this);
    }

private:
    RPP_NO_UNIQUE_ADDRESS Strategy m_strategy;
};
}
