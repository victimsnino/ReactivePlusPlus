//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/observables/details/member_overload.hpp> // override this
#include <rpp/observers/state_observer.hpp>            // proxy observer
#include <rpp/subscribers/constraints.hpp>             // concept for lift_impl
#include <rpp/utils/function_traits.hpp>               // extract subscriber type

namespace rpp::details
{
struct lift_tag;
}

IMPLEMENTATION_FILE(lift_tag);

namespace rpp::details
{
template<typename T, typename NewType>
concept lift_fn = constraint::subscriber<std::invoke_result_t<T, dynamic_subscriber<NewType>>>;

template<constraint::decayed_type Type, constraint::decayed_type OnNext, constraint::decayed_type OnError, constraint::decayed_type OnCompleted>
struct lift_action_by_callbacks
{
    [[no_unique_address]] OnNext      on_next;
    [[no_unique_address]] OnError     on_error;
    [[no_unique_address]] OnCompleted on_completed;

    template<constraint::subscriber TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  std::forward<TSub>(subscriber),
                                                  on_next,
                                                  on_error,
                                                  on_completed);
    }
};

template<typename... Types>
using decayed_lift_action_by_callbacks = lift_action_by_callbacks<std::decay_t<Types>...>;

template<constraint::decayed_type NewType, lift_fn<NewType> OperatorFn, typename TObs>
auto lift_action(TObs&& _this, OperatorFn&& op)
{
    return [_this = std::forward<TObs>(_this), op = std::forward<OperatorFn>(op)]<constraint::subscriber_of_type<NewType> TSub>(TSub&& subscriber)
    {
        _this.subscribe(op(std::forward<TSub>(subscriber)));
    };
}

template<constraint::decayed_type NewType, lift_fn<NewType> OperatorFn, typename TObs>
auto lift_impl(OperatorFn&& op, TObs&& _this) -> specific_observable<NewType, decltype(lift_action<NewType>(std::forward<TObs>(_this), std::forward<OperatorFn>(op)))>
{
    return {lift_action<NewType>(std::forward<TObs>(_this), std::forward<OperatorFn>(op))};
}

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, lift_tag>
{
    /**
    * \brief The lift operator provides ability to create your own operator and apply it to observable
    * \tparam NewType manually specified new type of observable after applying of fn
    * \param op represents operator logic in the form: accepts NEW subscriber and returns OLD subscriber
    * \return new specific_observable of NewType
    */
    template<constraint::decayed_type NewType>
    auto lift(lift_fn<NewType> auto&& op) const&
    {
        return details::lift_impl<NewType>(std::forward<decltype(op)>(op), CastThis());
    }

    template<constraint::decayed_type NewType>
    auto lift(lift_fn<NewType> auto&& op)&&
    {
        return details::lift_impl<NewType>(std::forward<decltype(op)>(op), MoveThis());
    }

    // ********************************* LIFT OPERATOR: SUBSCRIBER -> SUBSCRIBER ******************//
    /**
    * \brief The lift operator provides ability to create your own operator and apply it to observable
    * \tparam OperatorFn type of your custom functor
    * \tparam NewType auto-deduced type of observable after applying of fn
    * \param op represents operator logic in the form: accepts NEW subscriber and returns OLD subscriber
    * \return new specific_observable of NewType
    */
    template<typename OperatorFn,
             constraint::decayed_type NewType = utils::extract_subscriber_type_t<utils::function_argument_t<OperatorFn>>>
    auto lift(OperatorFn&& op) const& requires details::lift_fn<OperatorFn, NewType>
    {
        return details::lift_impl<NewType>(std::forward<decltype(op)>(op), CastThis());
    }
    template<typename OperatorFn,
             constraint::decayed_type NewType = utils::extract_subscriber_type_t<utils::function_argument_t<OperatorFn>>>
    auto lift(OperatorFn&& op) && requires details::lift_fn<OperatorFn, NewType>
    {
        return details::lift_impl<NewType>(std::forward<decltype(op)>(op), MoveThis());
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
             std::invocable<std::exception_ptr, dynamic_subscriber<NewType>> OnError     = forwarding_on_error,
             std::invocable<dynamic_subscriber<NewType>>                     OnCompleted = forwarding_on_completed>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const&
    {
        return details::lift_impl<NewType>(details::decayed_lift_action_by_callbacks<Type, OnNext, OnError, OnCompleted>{std::forward<OnNext>(on_next),
                                                                                                                         std::forward<OnError>(on_error),
                                                                                                                         std::forward<OnCompleted>(on_completed)},
                                           CastThis());
    }

    template<constraint::decayed_type                                        NewType,
             std::invocable<Type, dynamic_subscriber<NewType>>               OnNext,
             std::invocable<std::exception_ptr, dynamic_subscriber<NewType>> OnError     = forwarding_on_error,
             std::invocable<dynamic_subscriber<NewType>>                     OnCompleted = forwarding_on_completed>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {})&&
    {
        return details::lift_impl<NewType>(details::decayed_lift_action_by_callbacks<Type, OnNext, OnError, OnCompleted>{std::forward<OnNext>(on_next),
                                                                                                                         std::forward<OnError>(on_error),
                                                                                                                         std::forward<OnCompleted>(on_completed)},
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
             constraint::decayed_type                                        NewType = utils::extract_subscriber_type_t<std::decay_t<utils::function_argument_t<OnNext, 1>>>,
             std::invocable<std::exception_ptr, dynamic_subscriber<NewType>> OnError = forwarding_on_error,
             std::invocable<dynamic_subscriber<NewType>>                     OnCompleted = forwarding_on_completed>
        requires std::invocable<OnNext, Type, dynamic_subscriber<NewType>>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const&
    {
        return details::lift_impl<NewType>(details::decayed_lift_action_by_callbacks<Type, OnNext, OnError, OnCompleted>{std::forward<OnNext>(on_next),
                                                                                                                         std::forward<OnError>(on_error),
                                                                                                                         std::forward<OnCompleted>(on_completed)},
                                           CastThis());
    }

    template<typename                                                        OnNext,
             constraint::decayed_type                                        NewType = utils::extract_subscriber_type_t<std::decay_t<utils::function_argument_t<OnNext, 1>>>,
             std::invocable<std::exception_ptr, dynamic_subscriber<NewType>> OnError = forwarding_on_error,
             std::invocable<dynamic_subscriber<NewType>>                     OnCompleted = forwarding_on_completed>
        requires std::invocable<OnNext, Type, dynamic_subscriber<NewType>>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {})&&
    {
        return details::lift_impl<NewType>(details::decayed_lift_action_by_callbacks<Type, OnNext, OnError, OnCompleted>{std::forward<OnNext>(on_next),
                                                                                                                         std::forward<OnError>(on_error),
                                                                                                                         std::forward<OnCompleted>(on_completed)},
                                           MoveThis());
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
};
} // namespace rpp::details
