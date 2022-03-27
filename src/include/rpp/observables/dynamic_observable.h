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

#include <rpp/observables/interface_observable.h> // base class
#include <rpp/observables/specific_observable.h>  // m_observable
#include <rpp/observables/type_traits.h>          // extract_observable_type

#include <memory>

namespace rpp
{
/**
 * \brief type-erased alternative of observable (comparing to rpp::specific_observable).
 *
 * It uses type-erasure mechanism to hide type of OnSubscribeFn. But it has higher cost in the terms of performance due to usage of heap.
 * Use it only when you need to store observable as member variable or something like this
 * \tparam Type is type of value provided by this observable
 * \ingroup observables
 */
template<constraint::decayed_type Type>
class dynamic_observable final : public interface_observable<Type, dynamic_observable<Type>>
{
public:
    dynamic_observable(constraint::on_subscribe_fn<Type> auto&& on_subscribe)
        : m_observable{std::make_shared<specific_observable<Type, std::decay_t<decltype(on_subscribe)>>>(on_subscribe)} {}

    template<constraint::observable TObs>
        requires (!std::is_same_v<std::decay_t<TObs>, dynamic_observable<Type>>)
    dynamic_observable(TObs&& observable)
        : m_observable{ std::make_shared<std::decay_t<TObs>>(std::forward<TObs>(observable)) } {}

    dynamic_observable(const dynamic_observable<Type>&)     = default;
    dynamic_observable(dynamic_observable<Type>&&) noexcept = default;

    subscription subscribe(const dynamic_subscriber<Type>& subscriber) const noexcept override
    {
        return m_observable->subscribe(subscriber);
    }

    template<typename ...Args>
        requires (std::is_constructible_v<dynamic_subscriber<Type>, Args...> && !constraint::variadic_is_same_type<dynamic_subscriber<Type>, Args...>)
    subscription subscribe(Args&&...args) const noexcept
    {
        return m_observable->subscribe(dynamic_subscriber<Type>{std::forward<Args>(args)...});
    }

    const dynamic_observable<Type>& as_dynamic() const { return *this; }

private:
    std::shared_ptr<virtual_observable<Type>> m_observable{};
};

template<constraint::observable TObs>
dynamic_observable(TObs obs) -> dynamic_observable<utils::extract_observable_type_t<TObs>>;

template<typename OnSub>
dynamic_observable(OnSub on_subscribe) -> dynamic_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>>;
} // namespace rpp
