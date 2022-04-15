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

#include <rpp/observables/constraints.h>
#include <rpp/operators/fwd/merge.h>
#include <rpp/subscribers/constraints.h>
#include <rpp/subscriptions/composite_subscription.h>
#include <rpp/observers/state_observer.h>

#include <atomic>
#include <memory>

IMPLEMENTATION_FILE(merge_tag);

namespace rpp::operators
{
template<typename ...Args>
auto merge() requires details::is_header_included<details::merge_tag, Args...>
{
    return []<constraint::observable TObservable>(TObservable && observable)
    {
        return observable.merge();
    };
}
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type>
auto create_proxy_subscriber(const constraint::subscriber auto&         subscriber,
                             const std::shared_ptr<std::atomic_size_t>& on_completed_count,
                             auto&&                                     on_next)
{
    ++(*on_completed_count);

    auto subscription = subscriber.get_subscription().make_child();

    return create_subscriber_with_state<Type>(std::move(subscription),
                                              std::forward<decltype(subscriber)>(subscriber),
                                              std::forward<decltype(on_next)>(on_next),
                                              forwarding_on_error{},
                                              [=](const constraint::subscriber auto& sub)
                                              {
                                                  if (--(*on_completed_count) == 0)
                                                    sub.on_completed();
                                              });
};

template<constraint::decayed_type Type, typename SpecificObservable>
auto member_overload<Type, SpecificObservable, merge_tag>::merge_impl()
{
    return []<constraint::subscriber TSub>(TSub&& subscriber)
    {
        auto count_of_on_completed_required = std::make_shared<std::atomic_size_t>();

        auto on_new_observable = [=]<constraint::observable TObs>(TObs&& new_observable, const constraint::subscriber auto& sub)
        {
            using ValueType = utils::extract_observable_type_t<Type>;
            std::forward<TObs>(new_observable).subscribe(create_proxy_subscriber<ValueType>(sub, count_of_on_completed_required, forwarding_on_next{}));
        };

        return create_proxy_subscriber<Type>(subscriber, count_of_on_completed_required, std::move(on_new_observable));
    };
}
} // namespace rpp::details
