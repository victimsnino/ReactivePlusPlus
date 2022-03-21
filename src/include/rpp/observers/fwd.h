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

#include <rpp/utils/constraints.h>

namespace rpp::details
{
struct observer_tag;
} // namespace rpp::details

namespace rpp::constraint
{
template<typename Fn, typename Type, typename ...States> concept on_next_fn      = std::invocable<std::decay_t<Fn>, Type, States...>;
template<typename Fn, typename ...States>                concept on_error_fn     = std::invocable<std::decay_t<Fn>, std::exception_ptr, States...>;
template<typename Fn, typename ...States>                concept on_completed_fn = std::invocable<std::decay_t<Fn>, States...>;
} // namespace rpp::constraint

namespace rpp
{
template<constraint::decayed_type T>
struct interface_observer;

template<constraint::decayed_type Type>
class dynamic_observer;

template<constraint::decayed_type T,
         constraint::on_next_fn<T>   OnNext,
         constraint::on_error_fn     OnError,
         constraint::on_completed_fn OnCompleted>
class specific_observer;
} // namespace rpp