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

#include <rpp/fwd.h>

namespace rpp::utils
{
namespace details
{
template<typename Type, typename ...Args>
struct is_observer_constructible
        : std::false_type {};

template<typename Type, typename Fn1>
struct is_observer_constructible<Type, Fn1>
        : std::is_invocable<Fn1, Type> {};

template<typename Type, typename Fn1, typename Fn2>
struct is_observer_constructible<Type, Fn1, Fn2>
        : std::conjunction<std::is_invocable<Fn1, Type>,
                           std::disjunction<std::is_invocable<Fn2, std::exception_ptr>,
                                            std::is_invocable<Fn2>>> {};

template<typename Type, typename Fn1, typename Fn2, typename Fn3>
struct is_observer_constructible<Type, Fn1, Fn2, Fn3>
        : std::conjunction<std::is_invocable<Fn1, Type>,
                           std::is_invocable<Fn2, std::exception_ptr>,
                           std::is_invocable<Fn3>> {};
} // namespace details

template<typename Type, typename ...Args>
constexpr bool is_observer_constructible_v = details::is_observer_constructible<Type, std::decay_t<Args>...>::value;

template<typename Type, typename ...Args>
using enable_if_observer_constructible_t = std::enable_if_t<is_observer_constructible_v<Type, std::decay_t<Args>...>>;

// ************************ OBSERVABLE *********************** //
namespace details
{
    template<typename T>
    T extract_observable_type(const virtual_observable<T>&);
} // namespace details
template<typename T>
using extract_observable_type_t = decltype(details::extract_observable_type(std::declval<std::decay_t<T>>()));

template<typename T>
constexpr bool is_observable_v = std::is_base_of_v<rpp::details::observable_tag, std::decay_t<T>>;
} // namespace rpp::utils
