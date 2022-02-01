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

#include <rpp/subscription.h>
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
 * Is has better performance comparing to dynamic_observable. Use it if possible. But it has worse usability due to OnSubscribeFn template parameter.
 * \tparam Type is type of value provided by this observable
 * \tparam OnSubscribeFn is type of function/functor/callable used during subscription on this observable
 */
template<typename Type, typename OnSubscribeFn>
class specific_observable final : public interface_observable<Type, specific_observable<Type, OnSubscribeFn>>
{
    template<typename T>
    using enable_if_callable_t = std::enable_if_t<std::is_invocable_v<T, subscriber<Type>>>;

public:
    template<typename OnSubscribe,
             typename Enable = enable_if_callable_t<OnSubscribe>>
    specific_observable(OnSubscribe&& on_subscribe = {})
        : m_state{std::forward<OnSubscribe>(on_subscribe)} {}

    dynamic_observable<Type> as_dynamic() const
    {
        return *this;
    }
    
    subscription subscribe(const subscriber<Type>& observer) const override
    {
        try
        {
            m_state(observer);
        }
        catch (const std::exception& exc)
        {
            observer.on_error(std::make_exception_ptr(exc));
        }
        return observer.get_subscription();
    }

private:
    OnSubscribeFn m_state;
};

template<typename OnSub>
specific_observable(OnSub on_subscribe) -> specific_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>, OnSub>;
} // namespace rpp
