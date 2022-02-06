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

#include <rpp/observables/interface_observable.h>

#include <memory>

namespace rpp
{
/**
 * \brief type-erased alternative of observable (comparing to rpp::specific_observable).
 *
 * It uses type-erasure mechanism to hide type of OnSubscribeFn. But it has higher cost in the terms of performance due to usage of heap.
 * Use it only when you need to store observable as member variable or something like this
 * \tparam Type is type of value provided by this observable
 */
template<typename Type>
class dynamic_observable final : public interface_observable<Type, dynamic_observable<Type>>
{
public:
    template<typename OnSubscribeFn>
    dynamic_observable(const specific_observable<Type, OnSubscribeFn>& observable)
        : m_observable{ std::make_shared<specific_observable<Type, OnSubscribeFn>>(observable) } {}

    template<typename OnSubscribeFn>
    dynamic_observable(specific_observable<Type, OnSubscribeFn>&& observable)
        : m_observable{ std::make_shared<specific_observable<Type, OnSubscribeFn>>(std::move(observable)) } {}

    subscription subscribe(const subscriber<Type>& subscriber) const noexcept override { return m_observable->subscribe(subscriber); }

private:
    std::shared_ptr<virtual_observable<Type>> m_observable{};
};
} // namespace rpp
