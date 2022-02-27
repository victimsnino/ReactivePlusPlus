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
// *************************** SUBSCRIBER ************************//
template<typename>
struct extract_subscriber_type;

template<typename Type, typename Obs>
struct extract_subscriber_type<subscriber<Type, Obs>>
{
    using type = Type;
};

template<typename Type>
struct extract_subscriber_type : public extract_subscriber_type<std::decay_t<Type>>{};

template<typename T>
using extract_subscriber_type_t = typename extract_subscriber_type<T>::type;

template<typename T>
struct is_subscriber: std::false_type{};

template<typename T, typename Obs>
struct is_subscriber<subscriber<T, Obs>> : std::true_type{};

template<typename T>
constexpr bool is_subscriber_v = is_subscriber<T>::value;

// *************************** OBSERVER ************************//
template<typename T>
struct is_observer : std::false_type{};

template<typename T>
struct is_observer<dynamic_observer<T>> : std::true_type{};

template<typename T>
constexpr bool is_observer_v = is_observer<T>::value;

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
} // namespace rpp::utils
