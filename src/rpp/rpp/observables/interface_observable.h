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

#include "rpp/utils/type_traits.h"

#include <rpp/fwd.h>
#include <rpp/subscriber.h>

#include <type_traits>

namespace rpp
{
template<typename Type>
struct virtual_observable
{
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Type of observable should be decayed");

    virtual              ~virtual_observable() = default;
    virtual subscription subscribe(const subscriber<Type>& subscriber) const noexcept = 0;
};

template<typename Type, typename SpecificObservable>
struct interface_observable : public virtual_observable<Type>
{
    template<typename OperatorFn,
             typename SubscriberType = utils::function_argument_t<OperatorFn>,
             typename NewType = utils::extract_subscriber_type_t<SubscriberType>>
    auto lift(OperatorFn&& op) &
    {
        static_assert(utils::is_subscriber<typename utils::function_traits<OperatorFn>::result>{},
            "OperatorFn should return subscriber of same type");

        return specific_observable{[new_this = *CastThis(), op = std::forward<OperatorFn>(op)](SubscriberType subscriber)
        {
            new_this.subscribe(op(std::forward<SubscriberType>(subscriber)));
        }};
    }

    template<typename OperatorFn,
             typename SubscriberType = utils::function_argument_t<OperatorFn>,
             typename NewType = utils::extract_subscriber_type_t<SubscriberType>>
    auto lift(OperatorFn&& op) &&
    {
        static_assert(utils::is_subscriber<typename utils::function_traits<OperatorFn>::result>{},
            "OperatorFn should return copyable_subscriber of same type");

        return specific_observable{[new_this = std::move(*CastThis()),op = std::forward<OperatorFn>(op)](SubscriberType subscriber)
        {
            new_this.subscribe(op(std::forward<SubscriberType>(subscriber)));
        }};
    }

private:
    SpecificObservable* CastThis() {return static_cast<SpecificObservable*>(this);}
};
} // namespace rpp
