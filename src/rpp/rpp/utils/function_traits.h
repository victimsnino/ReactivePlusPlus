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

#include <tuple>

namespace rpp::utils
{
// Lambda
template<class T>
struct function_traits : function_traits<decltype(&T::operator())> {};

// Operator of lambda
template<class T, class R, class... Args>
struct function_traits<R (T::*)(Args ...) const> : function_traits<R(*)(Args ...)> {};

// Operator of lambda with mutable
template<class T, class R, class... Args>
struct function_traits<R (T::*)(Args ...)> : function_traits<R(*)(Args ...)> {};

// Classical global function
template<class R, class... Args>
struct function_traits<R (*)(Args ...)>
{
    using result = R;
    using arguments = std::tuple<Args...>;

    template<size_t i = 0>
    using argument = std::tuple_element_t<i, arguments>;
};

template<typename T, size_t i = 0>
using function_argument_t = typename function_traits<T>::template argument<i>;


template<typename T, typename = void>
struct is_callable : std::false_type{};

template<typename T>
struct is_callable<T, std::void_t<decltype(&T::operator())>> : std::true_type{};

template<class T, class R, class... Args>
struct is_callable<R (T::*)(Args ...) const> : std::true_type{};

template<class T, class R, class... Args>
struct is_callable<R (T::*)(Args ...)> : std::true_type{};

template<class R, class... Args>
struct is_callable<R (*)(Args ...)> : std::true_type{};

template<typename T>
constexpr bool is_callable_v = is_callable<T>::value;
} // namespace rpp::utils
