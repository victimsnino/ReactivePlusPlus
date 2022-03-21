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

#include <rpp/subscribers/specific_subscriber.h>

namespace rpp
{
/**
 * \brief subscriber which uses dynamic_observer<T> to hide original callbacks
 * \tparam T type of values expected by this subscriber
 */
template<constraint::decayed_type T>
class dynamic_subscriber final : public specific_subscriber<T, dynamic_observer<T>>
{
public:
    using specific_subscriber<T, dynamic_observer<T>>::specific_subscriber;

    template<typename Obs>
    dynamic_subscriber(const specific_subscriber<T, Obs>& subscriber)
        : specific_subscriber<T, dynamic_observer<T>>{subscriber.get_subscription(), subscriber.get_observer()} {}
};

template<constraint::observer TObs>
dynamic_subscriber(TObs observer) -> dynamic_subscriber<utils::extract_observer_type_t<TObs>>;

template<constraint::observer TObs>
dynamic_subscriber(subscription, TObs observer) -> dynamic_subscriber<utils::extract_observer_type_t<TObs>>;

template<typename T, typename Obs>
dynamic_subscriber(specific_subscriber<T, Obs>) -> dynamic_subscriber<T>;

template<typename OnNext, typename ...Args>
dynamic_subscriber(subscription, OnNext, Args...) -> dynamic_subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;

template<typename OnNext, typename ...Args>
dynamic_subscriber(OnNext, Args ...) -> dynamic_subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;
} // namespace rpp
