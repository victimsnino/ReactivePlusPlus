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

#include <rpp/defs.hpp>
#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/observables/details/chain_strategy.hpp>
#include <rpp/observers/lambda_observer.hpp>
#include <rpp/operators/subscribe.hpp>

namespace rpp
{
/**
 * @brief Base class for any observable used in RPP. It handles core callbacks of observable.
 * @details Observable provides only one core function: subscribe - it accepts observer (or any way to construct it) and then invokes underlying Strategy to emit emissions somehow.
 * @warning Actually observable "doesn't emit nothing", it only **invokes Strategy!** Strategy COULD emit emissions immediately OR place observer to some queue or something like this to obtain emissions later (for example subjects)
 * @warning Expected that observable's strategy would work with observer in serialized way
 *
 * @tparam Type of value this observable would provide. Only observers of same type can be subscribed to this observable.
 * @tparam Strategy used to provide logic over observable's callbacks.
 *
 * @ingroup observables
 */
template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class observable
{
public:
    using value_type = Type;

    using expected_disposable_strategy = rpp::details::observables::deduce_disposable_strategy_t<Strategy>;

    template<typename... Args>
        requires (!constraint::variadic_decayed_same_as<observable<Type, Strategy>, Args...> && constraint::is_constructible_from<Strategy, Args&& ...>)
    observable(Args&&... args)
        : m_strategy{std::forward<Args>(args)...}
    {
    }

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
     * @brief Subscribes passed observer strategy to emissions from this observable via construction of observer
     */
    template<constraint::observer_strategy<Type> ObserverStrategy>
        requires (!constraint::observer<ObserverStrategy>)
    void subscribe(ObserverStrategy&& observer_strategy) const
    {
        if constexpr (details::observers::has_disposable_strategy<ObserverStrategy>)
            subscribe(rpp::observer<Type, std::decay_t<ObserverStrategy>>{std::forward<ObserverStrategy>(observer_strategy)});
        else
            subscribe(rpp::observer_with_disposable<Type, std::decay_t<ObserverStrategy>, typename expected_disposable_strategy::disposable_strategy>{std::forward<ObserverStrategy>(observer_strategy)});
    }

    /**
     * @brief Subscribe passed observer to emissions from this observable.
     * @details This overloading attaches passed disposable to observer and return it to provide ability to dispose/disconnect observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     *
     * @param d is disposable to be attached to observer. If disposable is nullptr or disposed -> no any subscription happens
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     *
     * @par Example
     * \code{.cpp}
    *  auto disposable = rpp::composite_disposable_wrapper::make();
    *  rpp::source::just(1)
    *  | rpp::operators::repeat()
    *  | rpp::operators::subscribe_on(rpp::schedulers::new_thread{})
    *  | rpp::operators::subscribe(disposable, rpp::make_lambda_observer([](int) { std::cout << "NEW VALUE" << std::endl; }));
    *
    *  std::this_thread::sleep_for(std::chrono::seconds(1));
    *  disposable.dispose();
    *  std::this_thread::sleep_for(std::chrono::seconds(1));
    * \endcode
     *
     */
    template<constraint::observer_strategy<Type> ObserverStrategy>
    composite_disposable_wrapper subscribe(const composite_disposable_wrapper& d, observer<Type, ObserverStrategy>&& obs) const
    {
        if (!d.is_disposed())
            m_strategy.subscribe(observer_with_disposable<Type, observer<Type, ObserverStrategy>>{d, std::move(obs)});
        return d;
    }

    /**
     * @brief Subscribes passed observer strategy to emissions from this observable via construction of observer
     * @details This overloading attaches passed disposable to observer and return it to provide ability to dispose/disconnect observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     *
     * @param d is disposable to be attached to observer. If disposable is nullptr or disposed -> no any subscription happens
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     */
    template<constraint::observer_strategy<Type> ObserverStrategy>
        requires (!constraint::observer<ObserverStrategy>)
    composite_disposable_wrapper subscribe(const composite_disposable_wrapper& d, ObserverStrategy&& observer_strategy) const
    {
        subscribe(observer_with_disposable<Type, std::decay_t<ObserverStrategy>>{d, std::forward<ObserverStrategy>(observer_strategy)});
        return d;
    }

    /**
     * @brief Subscribes passed observer to emissions from this observable.
     *
     * @details This overloading attaches disposable to observer and return it to provide ability to dispose/disconnect observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     *
     * @warning Observer must be moved in to subscribe method. (Not recommended) If you need to copy observer, convert it to dynamic_observer
     */
    template<constraint::observer_strategy<Type> ObserverStrategy>
    [[nodiscard("Use returned disposable or use subscribe(observer) instead")]] composite_disposable_wrapper subscribe_with_disposable(observer<Type, ObserverStrategy>&& observer) const
    {
        if (!observer.is_disposed())
            return subscribe(rpp::composite_disposable_wrapper::make<rpp::composite_disposable_impl<typename expected_disposable_strategy::disposable_container>>(), std::move(observer));
        return composite_disposable_wrapper::empty();
    }

    /**
     * @brief Subscribes observer strategy to emissions from this observable.
     *
     * @details This overloading attaches disposable to observer and return it to provide ability to dispose/disconnect observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     */
   template<constraint::observer_strategy<Type> ObserverStrategy>
        requires (!constraint::observer<ObserverStrategy>)
    [[nodiscard("Use returned disposable or use subscribe(observer) instead")]] composite_disposable_wrapper subscribe_with_disposable(ObserverStrategy&& observer_strategy) const
    {
        return subscribe(rpp::composite_disposable_wrapper::make<rpp::composite_disposable_impl<typename expected_disposable_strategy::disposable_container>>(), std::forward<ObserverStrategy>(observer_strategy));
    }

    /**
     * @brief Subscribe passed observer to emissions from this observable.
     *
     * @details This overloading attaches disposable to observer and return it to provide ability to dispose/disconnect observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     *
     * @details Special overloading for dynamic observer to enable copy of observer
     */
    [[nodiscard("Use returned disposable or use subscribe(observer) instead")]] composite_disposable_wrapper subscribe_with_disposable(dynamic_observer<Type> observer) const
    {
        return subscribe<details::observers::dynamic_strategy<Type>>(rpp::composite_disposable_wrapper::make<rpp::composite_disposable_impl<typename expected_disposable_strategy::disposable_container>>(), std::move(observer));
    }

    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     */
    template<std::invocable<Type>                      OnNext,
             std::invocable<const std::exception_ptr&> OnError     = rpp::utils::rethrow_error_t,
             std::invocable<>                          OnCompleted = rpp::utils::empty_function_t<>>
    void subscribe(OnNext&&      on_next,
                   OnError&&     on_error     = {},
                   OnCompleted&& on_completed = {}) const
    {
        using strategy = rpp::details::observers::lambda_strategy<Type, std::decay_t<OnNext>, std::decay_t<OnError>, std::decay_t<OnCompleted>>;

        subscribe(observer_with_disposable<Type, strategy, typename expected_disposable_strategy::disposable_strategy>{std::forward<OnNext>(on_next),
                                                                                                                       std::forward<OnError>(on_error),
                                                                                                                       std::forward<OnCompleted>(on_completed)});
    }

    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     */
    template<std::invocable<Type>                      OnNext,
             std::invocable<>                          OnCompleted>
    void subscribe(OnNext&&      on_next,
                   OnCompleted&& on_completed) const
    {
       subscribe(std::forward<OnNext>(on_next), rpp::utils::rethrow_error_t{}, std::forward<OnCompleted>(on_completed));
    }

    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     *
     * @details This overloading attaches disposable to observer and return it to provide ability to dispose/disconnect observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     */
    template<std::invocable<Type>                      OnNext,
             std::invocable<const std::exception_ptr&> OnError     = rpp::utils::rethrow_error_t,
             std::invocable<>                          OnCompleted = rpp::utils::empty_function_t<>>
    [[nodiscard("Use returned disposable or use subscribe(on_next, on_error, on_completed) instead")]] composite_disposable_wrapper subscribe_with_disposable(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const
    {
        auto res = rpp::composite_disposable_wrapper::make<rpp::composite_disposable_impl<typename expected_disposable_strategy::disposable_container>>();
        subscribe(make_lambda_observer<Type>(res,
                                             std::forward<OnNext>(on_next),
                                             std::forward<OnError>(on_error),
                                             std::forward<OnCompleted>(on_completed)));
        return res;
    }

    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     *
     * @details This overloading attaches disposable to observer and return it to provide ability to dispose/disconnect observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     */
    template<std::invocable<Type>                      OnNext,
             std::invocable<>                          OnCompleted>
    [[nodiscard("Use returned disposable or use subscribe(on_next, on_error, on_completed) instead")]] composite_disposable_wrapper subscribe_with_disposable(OnNext&& on_next, OnCompleted&& on_completed) const
    {
        return subscribe_with_disposable(std::forward<OnNext>(on_next), rpp::utils::rethrow_error_t{}, std::forward<OnCompleted>(on_completed));
    }

    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     * @details This overloading attaches passed disposable to observer and return it to provide ability to dispose/disconnect observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     *
     * @param d is disposable to be attached to observer. If disposable is nullptr or disposed -> no any subscription happens
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     *
     * @par Example
     * \code{.cpp}
     *  auto disposable = rpp::composite_disposable_wrapper::make();
     *  rpp::source::just(1)
     *  | rpp::operators::repeat()
     *  | rpp::operators::subscribe_on(rpp::schedulers::new_thread{})
     *  | rpp::operators::subscribe(disposable, [](int) { std::cout << "NEW VALUE" << std::endl; });
     *
     *  std::this_thread::sleep_for(std::chrono::seconds(1));
     *  disposable.dispose();
     *  std::this_thread::sleep_for(std::chrono::seconds(1));
     * \endcode
     *
     */
    template<std::invocable<Type>                      OnNext,
             std::invocable<const std::exception_ptr&> OnError     = rpp::utils::rethrow_error_t,
             std::invocable<>                          OnCompleted = rpp::utils::empty_function_t<>>
    composite_disposable_wrapper subscribe(const composite_disposable_wrapper& d, OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const
    {
        if (!d.is_disposed())
            subscribe(make_lambda_observer<Type>(d,
                                                 std::forward<OnNext>(on_next),
                                                 std::forward<OnError>(on_error),
                                                 std::forward<OnCompleted>(on_completed)));
        return d;
    }

    /**
     * @brief Construct rpp::lambda_observer on the fly and subscribe it to emissions from this observable
     * @details This overloading attaches passed disposable to observer and return it to provide ability to dispose/disconnect observer early if needed.
     * @warning This overloading has some performance penalties, use it only when you really need to use disposable
     *
     * @param d is disposable to be attached to observer. If disposable is nullptr or disposed -> no any subscription happens
     * @return composite_disposable_wrapper is disposable to be able to dispose observer when it needed
     *
     * @par Example
     * \code{.cpp}
     *  auto disposable = rpp::composite_disposable_wrapper::make();
     *  rpp::source::just(1)
     *  | rpp::operators::repeat()
     *  | rpp::operators::subscribe_on(rpp::schedulers::new_thread{})
     *  | rpp::operators::subscribe(disposable, [](int) { std::cout << "NEW VALUE" << std::endl; });
     *
     *  std::this_thread::sleep_for(std::chrono::seconds(1));
     *  disposable.dispose();
     *  std::this_thread::sleep_for(std::chrono::seconds(1));
     * \endcode
     *
     */
    template<std::invocable<Type>                      OnNext,
             std::invocable<>                          OnCompleted>
    composite_disposable_wrapper subscribe(const composite_disposable_wrapper& d, OnNext&& on_next, OnCompleted&& on_completed) const
    {
        return subscribe(d, std::forward<OnNext>(on_next), rpp::utils::rethrow_error_t{}, std::forward<OnCompleted>(on_completed));
    }

    /**
     * @brief Convert observable to type-erased version
     */
    auto as_dynamic() const & { return rpp::dynamic_observable<Type>{*this}; }

    /**
     * @brief Convert observable to type-erased version
     */
    auto as_dynamic() && { return rpp::dynamic_observable<Type>{std::move(*this)}; }

    template<constraint::operator_base<Type> Op>
    auto operator|(Op&& op) const &
    {
        RPP_CHECK_IF_TRAIT_ASSERTS_SATISFIED(Op, Type)
        return inner_make_chain_operator(std::forward<Op>(op));
    }

    template<constraint::operator_base<Type> Op>
    auto operator|(Op&& op) &&
    {
        RPP_CHECK_IF_TRAIT_ASSERTS_SATISFIED(Op, Type)
        return std::move(*this).inner_make_chain_operator(std::forward<Op>(op));
    }

    template<constraint::operator_observable_transform<const observable&> Op>
    auto operator|(Op&& op) const &
    {
        return std::forward<Op>(op)(*this);
    }

    template<constraint::operator_observable_transform<observable&&> Op>
    auto operator|(Op&& op) &&
    {
        return std::forward<Op>(op)(std::move(*this));
    }

    template<typename... Args>
    auto operator|(const rpp::operators::details::subscribe_t<Args...>& op) const
    {
        return op(*this);
    }

    template<typename... Args>
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
    template<constraint::operator_chain<Type, expected_disposable_strategy> Op>
    auto inner_make_chain_operator(Op&& op) const &
    {
        return observable<typename std::decay_t<Op>::template operator_traits_for_upstream_type<Type>::result_type, make_chain_observable_t<std::decay_t<Op>, Strategy>>{std::forward<Op>(op), m_strategy};
    }

    template<constraint::operator_chain<Type, expected_disposable_strategy> Op>
    auto inner_make_chain_operator(Op&& op) &&
    {
        return observable<typename std::decay_t<Op>::template operator_traits_for_upstream_type<Type>::result_type, make_chain_observable_t<std::decay_t<Op>, Strategy>>{std::forward<Op>(op), std::move(m_strategy)};
    }

private:
    RPP_NO_UNIQUE_ADDRESS Strategy m_strategy;
};
}
