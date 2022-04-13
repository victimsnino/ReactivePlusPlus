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
#include <memory>

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
template<constraint::decayed_type Type, typename SpecificObservable>
auto member_overload<Type, SpecificObservable, take_tag>::take_impl(size_t count)
{
    struct state
    {
        state(size_t count) : count{count} {}
        
        const size_t       count;
        std::atomic_size_t sent_count{};
    };

    return [count]<constraint::subscriber TSub>(TSub&& subscriber)
    {
        auto action = [state = std::make_shared<state>(count)](auto&& value, const constraint::subscriber auto& subscriber)
        {
            const auto old_value = state->sent_count.fetch_add(1);
            if (old_value < state->count)
            {
                subscriber.on_next(std::forward<decltype(value)>(value));
                if (state->count - old_value == 1)
                    subscriber.on_completed();
            }
        };

        auto subscription = subscriber.get_subscription();

        return specific_subscriber<Type, state_observer<Type, std::decay_t<TSub>, std::decay_t<decltype(action)>>>
        {
            subscription,
            std::forward<TSub>(subscriber),
            std::move(action),
            forwarding_on_error{},
            forwarding_on_completed{}
        };
    };
}
} // namespace rpp::details
