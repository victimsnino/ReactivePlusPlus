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
#include <rpp/utils/function_traits.h>
#include <rpp/utils/type_traits.h>

#include <utility>

namespace rpp
{
template<typename Type, typename OnSubscribe>
class observable
{
public:

    static_assert(std::is_same_v<std::decay_t<utils::function_argument_t<OnSubscribe>>, subscriber<Type>>, "OnSubscribe should accept subscriber of the same type as observable");

    observable(OnSubscribe&& on_subscribe)
        : m_on_subscribe{std::move(on_subscribe)} {}

    observable(const OnSubscribe& on_subscribe)
        : m_on_subscribe{on_subscribe} {}

    void subscribe(const subscriber<Type>& observer) const
    {
        m_on_subscribe(observer);
    }

private:
    OnSubscribe m_on_subscribe;
};

template<typename OnSubscribe>
observable(OnSubscribe on_subscribe) -> observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSubscribe>>, OnSubscribe>;
} // namespace rpp
