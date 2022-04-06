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

#include <rpp/subscribers/fwd.h>
#include <rpp/utils/constraints.h>

namespace rpp::details
{
struct observable_tag;
} // namespace rpp::details

namespace rpp::constraint
{
template<typename Fn, typename T> concept on_subscribe_fn = std::invocable<std::decay_t<Fn>, dynamic_subscriber<T>>;
} // namespace rpp::constraint

namespace rpp
{
template<constraint::decayed_type Type>
struct virtual_observable;

template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
class specific_observable;

template<constraint::decayed_type Type>
class dynamic_observable;
} // namespace rpp
