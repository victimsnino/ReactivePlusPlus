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

#include <rpp/observables/specific_observable.h>

namespace rpp::observable
{
template<typename Type, typename OnSubscribeFn>
specific_observable<Type, std::remove_const_t<std::remove_reference_t<OnSubscribeFn>>> create(OnSubscribeFn&& on_subscribe)
{
    return {std::forward<OnSubscribeFn>(on_subscribe)};
}

template<typename OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe)
{
    return create<utils::extract_subscriber_type_t<utils::function_argument_t<OnSubscribeFn>>, OnSubscribeFn>(std::forward<OnSubscribeFn>(on_subscribe));
}
} // namespace rpp::observable