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

#include <concepts>
#include <exception>

#include <rpp/subscribers/fwd.h>

namespace rpp::details
{
struct observer_tag;
struct observable_tag;
struct subscriber_tag;
} // namespace rpp::details

namespace rpp::constraint
{
template<typename T, typename Type> concept decayed_same_as      = std::same_as<std::decay_t<T>, std::decay_t<Type>>;


template<typename Fn, typename Type> concept on_next_fn      = std::invocable<std::decay_t<Fn>, Type>;
template<typename Fn>                concept on_error_fn     = std::invocable<std::decay_t<Fn>, std::exception_ptr>;
template<typename Fn>                concept on_completed_fn = std::invocable<std::decay_t<Fn>>;

template<typename T> concept subscriber = std::is_base_of_v<details::subscriber_tag, std::decay_t<T>>;
template<typename T> concept observer   = std::is_base_of_v<details::observer_tag, std::decay_t<T>> && !subscriber<std::decay_t<T>>;
template<typename T> concept observable = std::is_base_of_v<details::observable_tag, std::decay_t<T>>;

template<typename Fn, typename T> concept on_subscribe_fn = std::invocable<std::decay_t<Fn>, dynamic_subscriber<T>>;
} // namespace rpp::constraint