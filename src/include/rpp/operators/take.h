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

#include <rpp/operators/fwd/take.h>
#include <rpp/observables/constraints.h>
#include <rpp/subscribers/constraints.h>

#include <atomic>

IMPLEMENTATION_FILE(take_tag);

namespace rpp::operators
{
template<typename...Args>
auto take(size_t count) requires details::is_header_included<details::take_tag, Args...>
{
    return [count]<constraint::observable TObservable>(TObservable&& observable)
    {
        return observable.take(count);
    };
}
} // namespace rpp::operators

namespace rpp::details
{
class take_action
{
public:
    take_action(size_t count)
        : m_count{count} {}

    take_action(const take_action& other)
        : m_count{other.m_count}
        , m_sent_count{} {}

    void operator()(auto&& value, const constraint::subscriber auto& subscriber) const
    {
        const auto old_value = m_sent_count.fetch_add(1);
        if (old_value < m_count)
        {
            subscriber.on_next(std::forward<decltype(value)>(value));
            if (m_count - old_value == 1)
                subscriber.on_completed();
        }
    }

private:
    const size_t               m_count;
    mutable std::atomic_size_t m_sent_count{};
};

template<constraint::decayed_type Type, typename SpecificObservable>
auto member_overload<Type, SpecificObservable, take_tag>::take_impl(size_t count)
{
    return take_action{count};
}
} // namespace rpp::details
