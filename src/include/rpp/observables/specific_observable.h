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

#include <rpp/observables/dynamic_observable.h>
#include <rpp/observables/interface_observable.h>
#include <rpp/utils/function_traits.h>
#include <rpp/utils/type_traits.h>

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
template<typename Type, typename OnSubscribeFn>
class specific_observable final : public interface_observable<Type, specific_observable<Type, OnSubscribeFn>>
{
    static_assert(std::is_same_v<std::decay_t<OnSubscribeFn>, OnSubscribeFn>, "OnSubscribeFn of specific_observable should be decayed");

public:
    specific_observable(const OnSubscribeFn& on_subscribe)
        : m_state{on_subscribe} {}

    specific_observable(OnSubscribeFn&& on_subscribe)
        : m_state{std::move(on_subscribe)} {}

    [[nodiscard]] dynamic_observable<Type> as_dynamic() const & { return *this;            }
    [[nodiscard]] dynamic_observable<Type> as_dynamic() &&      { return std::move(*this); }

     /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts dynamic_subscriber as most common and base type
     * \return subscription on this observable which can be used to unsubscribe
     */
    subscription subscribe(const dynamic_subscriber<Type>& subscriber) const noexcept override
    {
        return subscribe_impl(subscriber);
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts specific_subscriber to avoid construction of dynamic_subscriber
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<typename Obs>
    subscription subscribe(const specific_subscriber<Type, Obs>& subscriber) const noexcept
    {
        return subscribe_impl(subscriber);
    }

     /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts observer to construct specific_subscriber without extra overheads
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<typename TObserver, typename = std::enable_if_t<rpp::utils::is_observer_v<TObserver> && std::is_same_v<utils::extract_observer_type_t<TObserver>, Type>>>
    subscription subscribe(TObserver&& observer) const noexcept
    {
        return subscribe_impl<std::decay_t<TObserver>>(std::forward<TObserver>(observer));
    }

private:
    template<typename Obs>
    subscription subscribe_impl(const specific_subscriber<Type, Obs>& subscriber) const noexcept
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
