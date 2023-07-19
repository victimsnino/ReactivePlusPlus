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
#include <rpp/observables/details/chain_strategy.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/observers/dynamic_observer.hpp>
#include <rpp/observers/lambda_observer.hpp>

#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/operators/subscribe.hpp>

#include <rpp/defs.hpp>


namespace rpp
{
/**
 * @brief Base class for any observable used in RPP. It handles core callbacks of observable.
 * @details Observable provides ony core function: subscribe - it accepts observer (or any way to construct it) and then invokes underlying Strategy to emit emissions somehow.
 * @warning Actually observable "doesn't emit nothing", it only **invokes Strategy!** Strategy COULD emit emissions immediately OR place observer to some queue or something like this to obtain emissions later (for example subjects)
 *
 * @tparam Type of value this observable would provide. Only observers of same type can be subscribed to this observable.
 * @tparam Strategy used to provide logic over observable's callbacks.
 *
 * @ingroup observables
 */
template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class observable final
{
public:
    template<typename ...Args>
        requires (!constraint::variadic_decayed_same_as<observable<Type, Strategy>, Args...> && constraint::is_constructible_from<Strategy, Args&&...>)
    explicit observable(Args&& ...args)
        : m_strategy{std::forward<Args>(args)...} {}

    observable(const observable&)     = default;
    observable(observable&&) noexcept = default;

    /**
     * @brief Subscribes passed observer to emissions from this observable.
     *
     * @warning Observer must be moved in to subscribe method. (Not recommended) If you need to copy observer, convert it to dynamic_observer
     */
    template<constraint::observer_strategy<Type> ObserverStrategy>
    void subscribe(observer<Type, ObserverStrategy>&& observer) const
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
        subscribe<details::observers::dynamic_strategy<Type>>(std::move(observer));
    }

    /**
     * @brief Subscribe passed observer to emissions from this observable.
     * @details This overloading attaches passed disposable to observer and return it to provide ability to dispose observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     *
     * @param d is disposable to be attached to observer. If disposable is nullptr or disposed -> no any subscription happens
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     */
    template<constraint::observer_strategy<Type> ObserverStrategy>
    composite_disposable_wrapper subscribe(const composite_disposable_wrapper& d, observer<Type, ObserverStrategy>&& obs) const
    {   if (!d.is_disposed())
            m_strategy.subscribe(observer<Type, rpp::details::with_disposable<observer<Type, ObserverStrategy>>>{d, std::move(obs)});
        return d;
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
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     */
    template<std::invocable<Type>                      OnNext,
             std::invocable<const std::exception_ptr&> OnError     = rpp::utils::rethrow_error_t,
             std::invocable<>                          OnCompleted = rpp::utils::empty_function_t<>>
    composite_disposable_wrapper subscribe(const composite_disposable_wrapper& d, OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const
    {
        if (!d.is_disposed())
            m_strategy.subscribe(make_lambda_observer<Type>(d,
                                                std::forward<OnNext>(on_next),
                                                std::forward<OnError>(on_error),
                                                std::forward<OnCompleted>(on_completed)));
        return d;
    }

    /**
     * @brief Convert observable to type-erased versio
     */
    auto as_dynamic() const& { return rpp::dynamic_observable<Type>{*this}; }
    auto as_dynamic() && { return rpp::dynamic_observable<Type>{std::move(*this)}; }

    template<constraint::operators_v2<Type> Op>
    auto operator|(Op&& op) const &
    {
        return observable<typename std::decay_t<Op>::template ResultValue<Type>, make_chain_observable_t<std::decay_t<Op>, Strategy>>{std::forward<Op>(op), m_strategy};
    }

    template<constraint::operators_v2<Type> Op>
    auto operator|(Op&& op) &&
    {
        return observable<typename std::decay_t<Op>::template ResultValue<Type>, make_chain_observable_t<std::decay_t<Op>, Strategy>>{std::forward<Op>(op), std::move(m_strategy)};
    }

    template<constraint::operators<const observable<Type, Strategy>&> Op>
    auto operator|(Op&& op) const &
    {
        return std::forward<Op>(op)(*this);
    }

    template<constraint::operators<observable<Type, Strategy>&&> Op>
    auto operator|(Op&& op) &&
    {
        return std::forward<Op>(op)(std::move(*this));
    }

    template<typename...Args>
    auto operator|(const rpp::operators::details::subscribe_t<Args...>& op) const
    {
        return op(*this);
    }

    template<typename...Args>
    auto operator|(rpp::operators::details::subscribe_t<Args...>&& op) const
    {
        return std::move(op)(*this);
    }

    template<typename Op>
    auto pipe(Op&& op) const &
    {
        return *this | std::forward<Op>(op);
    }

    template<typename Op>
    auto pipe(Op&& op) &&
    {
        return std::move(*this) | std::forward<Op>(op);
    }

private:
    RPP_NO_UNIQUE_ADDRESS Strategy m_strategy;
};
}
