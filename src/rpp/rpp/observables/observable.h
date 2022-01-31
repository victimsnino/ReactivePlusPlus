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
#include <rpp/observables/observable_interface.h>
#include <rpp/utils/function_traits.h>
#include <rpp/utils/functors.h>
#include <rpp/utils/type_traits.h>

#include <utility>

namespace rpp
{
template<typename OnSubscribeFn>
class observable final : public observable_interface<utils::extract_subscriber_type_t<utils::function_argument_t<OnSubscribeFn>>, observable<OnSubscribeFn>>
{
    using Type = utils::extract_subscriber_type_t<utils::function_argument_t<OnSubscribeFn>>;

    template<typename T>
    using enable_if_callable_t = std::enable_if_t<std::is_invocable_v<T, subscriber<Type>>>;

public:
    template<typename OnSubscribe = utils::empty_functor<const subscriber<Type>&>,
             typename Enable = enable_if_callable_t<OnSubscribe>>
    observable(OnSubscribe&& on_subscribe = {})
        : m_state{std::forward<OnSubscribe>(on_subscribe)} {}

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
observable(OnSub on_subscribe) -> observable<OnSub>;
} // namespace rpp
