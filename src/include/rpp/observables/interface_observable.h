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

#include <rpp/observers/state_observer.h>

#include <rpp/fwd.h>
#include <rpp/subscriber.h>
#include <rpp/utils/type_traits.h>

#include <type_traits>

namespace rpp
{namespace details {
    template<typename NewType, typename OnNext, typename OnError, typename OnCompleted>
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
}

template<typename Type>
struct virtual_observable
{
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Type of observable should be decayed");

    virtual              ~virtual_observable() = default;
    virtual subscription subscribe(const dynamic_subscriber<Type>& subscriber) const noexcept = 0;
};

template<typename Type, typename SpecificObservable>
struct interface_observable : public virtual_observable<Type>
{
private:
    template<typename NewType,
             typename OperatorFn>
    static constexpr bool is_callable_returns_subscriber_of_same_type_v = std::is_same_v<Type, utils::extract_subscriber_type_t<std::invoke_result_t<OperatorFn, dynamic_subscriber<NewType>>>>;
public:
    // ********************************* LIFT DIRECT TYPE + OPERATOR: SUBSCRIBER -> SUBSCRIBER ******************//
    template<typename NewType,
             typename OperatorFn,
             typename = std::enable_if_t<std::is_invocable_v<OperatorFn, dynamic_subscriber<NewType>>>>
    auto lift(OperatorFn&& op) const &
    {
        return lift_impl<NewType>(std::forward<OperatorFn>(op), CastThis());
    }

    template<typename NewType,
             typename OperatorFn,
             typename = std::enable_if_t<std::is_invocable_v<OperatorFn, dynamic_subscriber<NewType>>>>
    auto lift(OperatorFn&& op) &&
    {
        return lift_impl<NewType>(std::forward<OperatorFn>(op), MoveThis());
    }

    // ********************************* LIFT OPERATOR: SUBSCRIBER -> SUBSCRIBER ******************//
    template<typename OperatorFn,
             typename SubscriberType = utils::function_argument_t<OperatorFn>,
             typename NewType = utils::extract_subscriber_type_t<SubscriberType>,
             typename = std::enable_if_t<std::is_invocable_v<OperatorFn, dynamic_subscriber<NewType>>>>
    auto lift(OperatorFn&& op) const &
    {
        return lift<NewType>(std::forward<OperatorFn>(op));
    }

    template<typename OperatorFn,
             typename SubscriberType = utils::function_argument_t<OperatorFn>,
             typename NewType = utils::extract_subscriber_type_t<SubscriberType>,
             typename = std::enable_if_t<std::is_invocable_v<OperatorFn, dynamic_subscriber<NewType>>>>
    auto lift(OperatorFn&& op) &&
    {
        return std::move(*this).template lift<NewType>(std::forward<OperatorFn>(op));
    }

    // ********************************* LIFT OnNext, Onerror, OnCompleted ******************//
    template<typename OnNext,
             typename OnError = details::forwarding_on_error,
             typename OnCompleted = details::forwarding_on_completed,
             typename NewType = std::decay_t<utils::function_argument_t<OnNext>>,
             typename = std::enable_if_t<std::is_invocable_v<OnNext, NewType, dynamic_subscriber<Type>>>>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const &
    {
        return lift<NewType>(std::forward<OnNext>(on_next),
                             std::forward<OnError>(on_error),
                             std::forward<OnCompleted>(on_completed));
    }

    template<typename OnNext,
             typename OnError = details::forwarding_on_error,
             typename OnCompleted = details::forwarding_on_completed,
             typename NewType = std::decay_t<utils::function_argument_t<OnNext>>,
             typename = std::enable_if_t<std::is_invocable_v<OnNext, NewType, dynamic_subscriber<Type>>>>
        auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) &&
    {
        return std::move(*this).template lift<NewType>(std::forward<OnNext>(on_next),
                                                       std::forward<OnError>(on_error),
                                                       std::forward<OnCompleted>(on_completed));
    }

    // ********************************* LIFT Direct type + OnNext, Onerror, OnCompleted ******************//

    template<typename NewType,
             typename OnNext,
             typename OnError = details::forwarding_on_error,
             typename OnCompleted = details::forwarding_on_completed,
             typename = std::enable_if_t<std::is_invocable_v<OnNext, NewType, dynamic_subscriber<Type>>>>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) const &
    {
        return lift_impl<NewType>(details::make_lift_action_by_callbacks<NewType>(std::forward<OnNext>(on_next),
                                                                                  std::forward<OnError>(on_error),
                                                                                  std::forward<OnCompleted>(on_completed)),
                                  CastThis());
    }

    template<typename NewType,
             typename OnNext,
             typename OnError = details::forwarding_on_error,
             typename OnCompleted = details::forwarding_on_completed,
             typename = std::enable_if_t<std::is_invocable_v<OnNext, NewType, dynamic_subscriber<Type>>>>
    auto lift(OnNext&& on_next, OnError&& on_error = {}, OnCompleted&& on_completed = {}) &&
    {
        return lift_impl<NewType>(details::make_lift_action_by_callbacks<NewType>(std::forward<OnNext>(on_next),
                                                                                  std::forward<OnError>(on_error),
                                                                                  std::forward<OnCompleted>(on_completed)),
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

    template<typename NewType,
             typename OperatorFn,
             typename FwdThis>
    static auto lift_impl(OperatorFn&& op, FwdThis&& _this)
    {
        static_assert(is_callable_returns_subscriber_of_same_type_v<NewType, OperatorFn>, "OperatorFn should return subscriber");

        return rpp::make_specific_observable<NewType>([new_this = std::forward<FwdThis>(_this), op = std::forward<OperatorFn>(op)](auto&& subscriber)
        {
            new_this.subscribe(op(std::forward<decltype(subscriber)>(subscriber)));
        });
    }
};
} // namespace rpp
