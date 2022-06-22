//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <tuple>

namespace rpp::utils
{
template<typename T, typename = void>
struct is_callable_t : std::false_type{};

template<typename T>
struct is_callable_t<T, std::void_t<decltype(&T::operator())>> : std::true_type{};

template<class T, class R, class... Args>
struct is_callable_t<R (T::*)(Args ...) const> : std::true_type{};

template<class T, class R, class... Args>
struct is_callable_t<R (T::*)(Args ...)> : std::true_type{};

template<class R, class... Args>
struct is_callable_t<R (*)(Args ...)> : std::true_type{};

template<typename T>
concept is_callable = is_callable_t<T>::value;

// Lambda
template<is_callable T>
struct function_traits : function_traits<decltype(&T::operator())> {};

// Operator of lambda
template<class T, class R, class... Args>
struct function_traits<R (T::*)(Args ...) const> : function_traits<R(*)(Args ...)> {};

// Operator of lambda with mutable
template<class T, class R, class... Args>
struct function_traits<R (T::*)(Args ...)> : function_traits<R(*)(Args ...)> {};

// Classical global function no args
template<class R>
struct function_traits<R (*)()>
{
    using result = R;
};

// Classical global function
template<class R, class... Args>
struct function_traits<R (*)(Args ...)>
{
    using result = R;
    using arguments = std::tuple<Args...>;

    template<size_t i = 0>
        requires (sizeof...(Args) > i)
    using argument = std::tuple_element_t<i, arguments>;
};

template<typename T, size_t i = 0>
using function_argument_t = typename function_traits<T>::template argument<i>;


template<typename T, size_t i = 0>
using decayed_function_argument_t = std::decay_t<function_argument_t<T, i>>;

template<typename Fn, typename ...Args>
using decayed_invoke_result_t = std::decay_t<std::invoke_result_t<Fn, Args...>>;

} // namespace rpp::utils
