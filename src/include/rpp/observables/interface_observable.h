// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <rpp/fwd.h>
#include <rpp/subscriber.h>
#include <rpp/observers/state_observer.h>
#include <rpp/utils/type_traits.h>

#include <type_traits>

namespace rpp
{
namespace details
{
    template<constraint::decayed_type NewType, typename OnNext, typename OnError, typename OnCompleted>
    static auto make_lift_action_by_callbacks(OnNext&& on_next, OnError&& on_error, OnCompleted&& on_completed)
    {
        return [on_next = std::forward<OnNext>(on_next),
                on_error = std::forward<OnError>(on_error),
                on_completed = std::forward<OnCompleted>(on_completed)](auto&& subscriber)
        {
            using SubType = decltype(subscriber);
            auto subscription = subscriber.get_subscription();
            return specific_subscriber
            {
                subscription,
                rpp::details::state_observer<std::decay_t<NewType>, std::decay_t<SubType>, std::decay_t<OnNext>, std::decay_t<OnError>, std::decay_t<OnCompleted>>
                {
                    std::forward<SubType>(subscriber),
                    on_next,
                    on_error,
                    on_completed
                }
            };
        };
    }

struct observable_tag{};


template<typename T, typename NewType>
concept lift_fn = constraint::subscriber<std::invoke_result_t<T, dynamic_subscriber<NewType>>>;

} // namespace details


/**
 * \brief Interface of observable
 * \tparam Type type provided by this observable
 */
template<constraint::decayed_type Type>
struct virtual_observable : public details::observable_tag
{
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Type of observable should be decayed");

    virtual              ~virtual_observable() = default;

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \return subscription on this observable which can be used to unsubscribe
     */
    virtual subscription subscribe(const dynamic_subscriber<Type>& subscriber) const noexcept = 0;
};

/**
 * \brief Base part of observable. Mostly used to provide some interface functions used by all observables
 * \tparam Type type provided by this observable 
 * \tparam SpecificObservable final type of observable inherited from this observable to successfully copy/move it
 */
template<typename Type, typename SpecificObservable>
struct interface_observable : public virtual_observable<Type>
{
private:
    template<typename NewType, typename OperatorFn>
    static constexpr bool is_callable_returns_subscriber_of_same_type_v = std::is_same_v<Type, utils::extract_subscriber_type_t<std::invoke_result_t<OperatorFn, dynamic_subscriber<NewType>>>>;
public:
    // ********************************* LIFT DIRECT TYPE + OPERATOR: SUBSCRIBER -> SUBSCRIBER ******************//
    /**
     * \brief Lift provides function to make new observable based on this one
     * \tparam NewType manually specified new type of observable after lift
     * \param op function applied to observable
     * \return new specific_observable of NewType
     */
    template<typename NewType>
    auto lift(details::lift_fn<NewType> auto&& op) const &
    {
        return lift_impl<NewType>(std::forward<decltype(op)>(op), CastThis());
    }

    /**
    * \brief Lift provides function to make new observable based on this one
    * \tparam NewType manually specified new type of observable after lift
    * \param op function applied to observable
    * \return new specific_observable of NewType
    */
    template<typename NewType>
    auto lift(details::lift_fn<NewType> auto&& op) &&
    {
        return lift_impl<NewType>(std::forward<decltype(op)>(op), MoveThis());
    }

    // ********************************* LIFT OPERATOR: SUBSCRIBER -> SUBSCRIBER ******************//
    /**
     * \brief Lift provides function to make new observable based on this one
     * \tparam OperatorFn type of function applied to observable
     * \tparam NewType deduced from argument of Operator of subscriber
     * \return new specific_observable of NewType
     */
    template<typename OperatorFn,
             typename SubscriberType = utils::function_argument_t<OperatorFn>,
             typename NewType = utils::extract_subscriber_type_t<SubscriberType>>
        requires details::lift_fn<OperatorFn, NewType>
    auto lift(OperatorFn&& op) const &
    {
        return lift<NewType>(std::forward<OperatorFn>(op));
    }

     /**
     * \brief Lift provides function to make new observable based on this one
     * \tparam OperatorFn type of function applied to observable
     * \tparam NewType deduced from argument of Operator of subscriber
     * \return new specific_observable of NewType
     */
    template<typename OperatorFn,
             typename SubscriberType = utils::function_argument_t<OperatorFn>,
             typename NewType = utils::extract_subscriber_type_t<SubscriberType>>
        requires details::lift_fn<OperatorFn, NewType>
    auto lift(OperatorFn&& op) &&
    {
        return std::move(*this).template lift<NewType>(std::forward<OperatorFn>(op));
    }

        // ********************************* LIFT Direct type + OnNext, Onerror, OnCompleted ******************//

    /**
     * \brief Lift observable via creation of new proxy subscriber with provided OnNext/OnError/OnCompleted callbacks
     * \tparam NewType manually specified new type of observable after lift
     * \tparam OnNext on_next of new subscriber
     * \tparam OnError on_error of new subscriber
     * \tparam OnCompleted on_completed of new subscriber
     * \return new specific_observable of NewType
     */
    template<typename                                                        NewType,
             std::invocable<Type, dynamic_subscriber<NewType>>               OnNext,
             std::invocable<std::exception_ptr, dynamic_subscriber<NewType>> OnError = details::forwarding_on_error,
             std::invocable<dynamic_subscriber<NewType>>                     OnCompleted = details::forwarding_on_completed>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const &
    {
        return lift_impl<NewType>(details::make_lift_action_by_callbacks<Type>(std::forward<OnNext>(on_next),
                                                                               std::forward<OnError>(on_error),
                                                                               std::forward<OnCompleted>(on_completed)),
                                  CastThis());
    }

    /**
     * \brief Lift observable via creation of new proxy subscriber with provided OnNext/OnError/OnCompleted callbacks
     * \tparam NewType manually specified new type of observable after lift
     * \tparam OnNext on_next of new subscriber
     * \tparam OnError on_error of new subscriber
     * \tparam OnCompleted on_completed of new subscriber
     * \return new specific_observable of NewType
     */
        template<typename                                                    NewType,
             std::invocable<Type, dynamic_subscriber<NewType>>               OnNext,
             std::invocable<std::exception_ptr, dynamic_subscriber<NewType>> OnError = details::forwarding_on_error,
             std::invocable<dynamic_subscriber<NewType>>                     OnCompleted = details::forwarding_on_completed>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) &&
    {
        return lift_impl<NewType>(details::make_lift_action_by_callbacks<Type>(std::forward<OnNext>(on_next),
                                                                               std::forward<OnError>(on_error),
                                                                               std::forward<OnCompleted>(on_completed)),
                                  MoveThis());
    }

    // ********************************* LIFT OnNext, Onerror, OnCompleted ******************//

    /**
     * \brief Lift observable via creation of new proxy subscriber with provided OnNext/OnError/OnCompleted callbacks
     * \tparam OnNext on_next of new subscriber
     * \tparam OnError on_error of new subscriber
     * \tparam OnCompleted on_completed of new subscriber
     * \tparam NewType type of new observable deduced by type of value used inside OnNext
     * \return new specific_observable of NewType
     */
    template<typename OnNext,
             typename OnError = details::forwarding_on_error,
             typename OnCompleted = details::forwarding_on_completed,
             typename NewType = std::decay_t<utils::function_argument_t<OnNext>>>
        requires std::invocable<OnNext, Type, dynamic_subscriber<NewType>> &&
                 std::invocable<OnError, std::exception_ptr, dynamic_subscriber<NewType>> &&
                 std::invocable<OnCompleted, dynamic_subscriber<NewType>>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const &
    {
        return lift<NewType>(std::forward<OnNext>(on_next),
                             std::forward<OnError>(on_error),
                             std::forward<OnCompleted>(on_completed));
    }

    /**
     * \brief Lift observable via creation of new proxy subscriber with provided OnNext/OnError/OnCompleted callbacks
     * \tparam OnNext on_next of new subscriber
     * \tparam OnError on_error of new subscriber
     * \tparam OnCompleted on_completed of new subscriber
     * \tparam NewType type of new observable deduced by type of value used inside OnNext
     * \return new specific_observable of NewType
     */
    template<typename OnNext,
             typename OnError = details::forwarding_on_error,
             typename OnCompleted = details::forwarding_on_completed,
             typename NewType = std::decay_t<utils::function_argument_t<OnNext>>>
        requires std::invocable<OnNext, Type, dynamic_subscriber<NewType>> &&
                 std::invocable<OnError, std::exception_ptr, dynamic_subscriber<NewType>> &&
                 std::invocable<OnCompleted, dynamic_subscriber<NewType>>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) &&
    {
        return std::move(*this).template lift<NewType>(std::forward<OnNext>(on_next),
                                                       std::forward<OnError>(on_error),
                                                       std::forward<OnCompleted>(on_completed));
    }
    
private:

    const SpecificObservable& CastThis() const
    {
        return *static_cast<const SpecificObservable*>(this);
    }

    SpecificObservable&& MoveThis()
    {
        return std::move(*static_cast<SpecificObservable*>(this));
    }

    template<typename NewType, typename OperatorFn, typename FwdThis>
    static auto lift_impl(OperatorFn&& op, FwdThis&& _this)
    {
        static_assert(is_callable_returns_subscriber_of_same_type_v<NewType, OperatorFn>, "OperatorFn should return subscriber");

        return rpp::observable::create<NewType>([new_this = std::forward<FwdThis>(_this), op = std::forward<OperatorFn>(op)](auto&& subscriber)
        {
            new_this.subscribe(op(std::forward<decltype(subscriber)>(subscriber)));
        });
    }
};

template<constraint::observable                  Observable,
        std::invocable<std::decay_t<Observable>> Operator>
auto operator |(Observable&& observable, Operator&& op)
{
    return std::forward<Operator>(op)(std::forward<Observable>(observable));
}
} // namespace rpp
