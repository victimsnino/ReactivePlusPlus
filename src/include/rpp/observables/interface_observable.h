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

#include <rpp/observables/constraints.h>        // own constraints
#include <rpp/observables/fwd.h>                // own declarations
#include <rpp/observers/state_observer.h>       // for make_lift_action_by_callbacks
#include <rpp/subscribers/constraints.h>        // for lift
#include <rpp/subscribers/specific_subscriber.h>// for make_lift_action_by_callbacks
#include <rpp/utils/constraints.h>              // general constraints

#include <type_traits>

namespace rpp::details
{
template<constraint::decayed_type Type, typename OnNext, typename OnError, typename OnCompleted>
static auto make_lift_action_by_callbacks(OnNext&& on_next, OnError&& on_error, OnCompleted&& on_completed)
{
    return [on_next = std::forward<OnNext>(on_next),
            on_error = std::forward<OnError>(on_error),
            on_completed = std::forward<OnCompleted>(on_completed)]<constraint::subscriber TSub>(TSub&& subscriber)
    {
        auto subscription = subscriber.get_subscription();
        return specific_subscriber<Type, state_observer<Type,
                                                        std::decay_t<TSub>,
                                                        std::decay_t<OnNext>,
                                                        std::decay_t<OnError>,
                                                        std::decay_t<OnCompleted>>>
        {
            subscription,
            std::forward<TSub>(subscriber),
            on_next,
            on_error,
            on_completed
        };
    };
}

struct observable_tag {};


template<typename T, typename NewType>
concept lift_fn = constraint::subscriber<std::invoke_result_t<T, dynamic_subscriber<NewType>>>;

template<typename T, typename TObservable>
concept op_fn = constraint::observable<std::invoke_result_t<T, TObservable>>;
} // namespace rpp::details

namespace rpp
{
/**
 * \defgroup observables Observables
 * \brief Observable is the source of any Reactive Stream. Observable provides ability to subscribe observer on some events.
 * \see https://reactivex.io/documentation/observable.html
 * 
 * 
 * \brief Interface of observable
 * \tparam Type type provided by this observable
 */
template<constraint::decayed_type Type>
struct virtual_observable : public details::observable_tag
{
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
template<constraint::decayed_type Type, typename SpecificObservable>
struct interface_observable : public virtual_observable<Type>
{
public:
    // ********************************* LIFT DIRECT TYPE + OPERATOR: SUBSCRIBER -> SUBSCRIBER ******************//
    /**
    * \brief The lift operator provides ability to create your own operator and apply it to observable
    * \tparam NewType manually specified new type of observable after applying of fn
    * \param fn represents operator logic in the form: accepts NEW subscriber and returns OLD subscriber
    * \return new specific_observable of NewType
    */
    template<constraint::decayed_type NewType>
    auto lift(details::lift_fn<NewType> auto&& op) const &
    {
        return lift_impl<NewType>(std::forward<decltype(op)>(op), CastThis());
    }

    template<constraint::decayed_type NewType>
    auto lift(details::lift_fn<NewType> auto&& op) &&
    {
        return lift_impl<NewType>(std::forward<decltype(op)>(op), MoveThis());
    }

    // ********************************* LIFT OPERATOR: SUBSCRIBER -> SUBSCRIBER ******************//
    /**
    * \brief The lift operator provides ability to create your own operator and apply it to observable
    * \tparam OperatorFn type of your custom functor
    * \tparam NewType auto-deduced type of observable after applying of fn
    * \param fn represents operator logic in the form: accepts NEW subscriber and returns OLD subscriber
    * \return new specific_observable of NewType
	*/
    template<typename OperatorFn,
             constraint::decayed_type NewType = utils::extract_subscriber_type_t<utils::function_argument_t<OperatorFn>>>
    auto lift(OperatorFn&& op) const & requires details::lift_fn<OperatorFn, NewType>
    {
        return lift<NewType>(std::forward<OperatorFn>(op));
    }
    template<typename OperatorFn,
             constraint::decayed_type NewType = utils::extract_subscriber_type_t<utils::function_argument_t<OperatorFn>>>
    auto lift(OperatorFn&& op) && requires details::lift_fn<OperatorFn, NewType>
    {
        return std::move(*this).template lift<NewType>(std::forward<OperatorFn>(op));
    }

        // ********************************* LIFT Direct type + OnNext, Onerror, OnCompleted ******************//

    /**
    * \brief The lift operator provides ability to create your own operator and apply it to observable.
    * \details This overload provides this ability via providing on_next, on_eror and on_completed with 2 params: old type of value + new subscriber
    * \tparam NewType manually specified new type of observable after lift
    * \tparam OnNext on_next of new subscriber accepting old value + new subscriber (logic how to transfer old value to new subscriber)
    * \tparam OnError on_error of new subscriber accepting exception  + new subscriber
    * \tparam OnCompleted on_completed of new subscriber accepting new subscriber
    * \return new specific_observable of NewType
    */
    template<constraint::decayed_type                                        NewType,
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

    template<constraint::decayed_type                                        NewType,
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
    * \brief The lift operator provides ability to create your own operator and apply it to observable.
    * \details This overload provides this ability via providing on_next, on_eror and on_completed with 2 params: old type of value + new subscriber
    * \tparam OnNext on_next of new subscriber accepting old value + new subscriber
    * \tparam OnError on_error of new subscriber accepting exception  + new subscriber
    * \tparam OnCompleted on_completed of new subscriber accepting new subscriber
    * \return new specific_observable of NewType
    */
    template<typename                                                        OnNext,
             constraint::decayed_type                                        NewType = std::decay_t<utils::function_argument_t<OnNext>>,
             std::invocable<std::exception_ptr, dynamic_subscriber<NewType>> OnError = details::forwarding_on_error,
             std::invocable<dynamic_subscriber<NewType>>                     OnCompleted = details::forwarding_on_completed>
        requires std::invocable<OnNext, Type, dynamic_subscriber<NewType>>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const &
    {
        return lift<NewType>(std::forward<OnNext>(on_next),
                             std::forward<OnError>(on_error),
                             std::forward<OnCompleted>(on_completed));
    }

    template<typename                                                        OnNext,
             constraint::decayed_type                                        NewType = std::decay_t<utils::function_argument_t<OnNext>>,
             std::invocable<std::exception_ptr, dynamic_subscriber<NewType>> OnError = details::forwarding_on_error,
             std::invocable<dynamic_subscriber<NewType>>                     OnCompleted = details::forwarding_on_completed>
        requires std::invocable<OnNext, Type, dynamic_subscriber<NewType>>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) &&
    {
        return std::move(*this).template lift<NewType>(std::forward<OnNext>(on_next),
                                                       std::forward<OnError>(on_error),
                                                       std::forward<OnCompleted>(on_completed));
    }

    /**
    * \brief The apply function to observable which returns observable of another type
    * \tparam OperatorFn type of function which applies to this observable
    * \return new specific_observable of NewType
    */
    template<details::op_fn<SpecificObservable> OperatorFn>
    auto op(OperatorFn&& fn) const &
    {
        return fn(CastThis());
    }

    template<details::op_fn<SpecificObservable> OperatorFn>
    auto op(OperatorFn&& fn) &&
    {
        return fn(MoveThis());
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

    template<constraint::decayed_type NewType, details::lift_fn<NewType> OperatorFn, typename FwdThis>
    static auto lift_impl(OperatorFn&& op, FwdThis&& _this)
    {
        auto action = [new_this = std::forward<FwdThis>(_this), op = std::forward<OperatorFn>(op)](auto&& subscriber)
        {
            new_this.subscribe(op(std::forward<decltype(subscriber)>(subscriber)));
        };

        return specific_observable<NewType, decltype(action)>(std::move(action));
    }
};

template<constraint::observable              Observable,
    details::op_fn<Observable> Operator>
auto operator |(Observable&& observable, Operator&& op)
{
    return std::forward<Observable>(observable).op(std::forward<Operator>(op));
}
} // namespace rpp
