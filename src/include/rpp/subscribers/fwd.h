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

#include <rpp/observers/fwd.h>

namespace rpp::details
{
struct subscriber_tag;

template<constraint::decayed_type Type>
class subscriber_base;
} // namespace rpp::details

namespace rpp::constraint
{
template<typename T> concept subscriber = std::is_base_of_v<details::subscriber_tag, std::decay_t<T>>;

template<typename T> concept decayed_observer = observer<T> && decayed_type<T>;
} // namespace rpp::constraint

namespace rpp
{
template<constraint::decayed_type Type, constraint::decayed_observer Observer>
class specific_subscriber;

template<constraint::decayed_type Type>
class dynamic_subscriber;
} // namespace rpp
