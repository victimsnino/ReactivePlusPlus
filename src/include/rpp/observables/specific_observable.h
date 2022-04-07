// MIT License
// 
// Copyright (c) 2021 Aleksey Loginov
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

#include <rpp/subscribers.h>                       // subscribe with all types
#include <rpp/observables/interface_observable.h> // base_class


#include <utility>

namespace rpp
{
/**
 * \brief observable specified with specific type of OnSubscribeFn. Used to store OnSubscribeFn function as is on stack (instead of allocating it somewhere).
 *
 * It has better performance comparing to rpp::dynamic_observable. Use it if possible. But it has worse usability due to OnSubscribeFn template parameter.
 * \tparam Type is type of value provided by this observable
 * \tparam OnSubscribeFn is type of function/functor/callable used during subscription on this observable
 * \ingroup observables
 */
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
class specific_observable final : public interface_observable<Type, specific_observable<Type, OnSubscribeFn>>
{
public:
    specific_observable(constraint::decayed_same_as<OnSubscribeFn> auto&& on_subscribe)
        : m_state{std::forward<decltype(on_subscribe)>(on_subscribe)} {}

    /**
     * \brief Converts rpp::specific_observable to rpp::dynamic_observable via type-erasure mechanism.
     */
    [[nodiscard]] auto as_dynamic() const & { return rpp::dynamic_observable<Type>{*this};            }
    [[nodiscard]] auto as_dynamic() &&      { return rpp::dynamic_observable<Type>{std::move(*this)}; }

    composite_subscription subscribe(const dynamic_subscriber<Type>& subscriber) const noexcept override
    {
        return subscribe_impl(subscriber);
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts subscriber as is to avoid construction of dynamic_subscriber
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::subscriber_of_type<Type> TSub>
        requires (!constraint::decayed_same_as<TSub, dynamic_subscriber<Type>>)
    composite_subscription subscribe(TSub&& subscriber) const noexcept
    {
        return subscribe_impl(subscriber);
    }

     /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts observer to construct specific_subscriber without extra overheads
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::observer_of_type<Type> TObserver>
    composite_subscription subscribe(TObserver&& observer) const noexcept
    {
        return subscribe_impl<std::decay_t<TObserver>>(std::forward<TObserver>(observer));
    }

    template<constraint::observer_of_type<Type> TObserver>
    composite_subscription subscribe(constraint::decayed_same_as<composite_subscription> auto&& sub, TObserver&& observer) const noexcept
    {
        return subscribe_impl(specific_subscriber<Type, std::decay_t<TObserver>>{std::forward<decltype(sub)>(sub), std::forward<TObserver>(observer)});
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts raw functions to construct specific subscriber with specific observer
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<typename ...Args>
        requires (std::is_constructible_v<dynamic_subscriber<Type>, std::decay_t<Args>...> && !constraint::variadic_is_same_type<dynamic_subscriber<Type>, Args...>)
    composite_subscription subscribe(Args&&...args) const noexcept
    {
        return subscribe_impl(rpp::make_specific_subscriber<Type>(std::forward<Args>(args)...));
    }

private:
    template<constraint::observer_of_type<Type> Obs>
    composite_subscription subscribe_impl(const specific_subscriber<Type, Obs>& subscriber) const noexcept
    {
        try
        {
            m_state(subscriber);
        }
        catch (const std::exception& exc)
        {
            subscriber.on_error(std::make_exception_ptr(exc));
        }
        return subscriber.get_subscription();
    }
private:
    OnSubscribeFn m_state;
};

template<typename OnSub>
specific_observable(OnSub on_subscribe) -> specific_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>, OnSub>;
} // namespace rpp
