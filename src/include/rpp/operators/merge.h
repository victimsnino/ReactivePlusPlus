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
auto create_proxy_subscriber(constraint::subscriber auto&&              subscriber,
                             const std::shared_ptr<std::atomic_size_t>& on_completed_count,
                             auto&&                                     on_next)
{
    ++(*on_completed_count);

    auto result = create_subscriber_with_state<Type>(std::forward<decltype(subscriber)>(subscriber),
                                                     std::forward<decltype(on_next)>(on_next),
                                                     forwarding_on_error{},
                                                     [=](const constraint::subscriber auto& sub)
                                                     {
                                                         if (--(*on_completed_count) == 0)
                                                             sub.on_completed();
                                                     });

    subscriber.get_subscription().add(result.get_subscription());
    return result;
};

template<constraint::decayed_type Type, typename SpecificObservable>
auto member_overload<Type, SpecificObservable, merge_tag>::merge_impl()
{
    using ValueType = utils::extract_observable_type_t<Type>;

    return []<constraint::subscriber_of_type<ValueType> TSub>(TSub&& subscriber)
    {
        auto count_of_on_completed_required = std::make_shared<std::atomic_size_t>();

        auto on_new_observable = [=]<constraint::observable TObs>(TObs&& new_observable, const constraint::subscriber auto& sub)
        {
            std::forward<TObs>(new_observable).subscribe(create_proxy_subscriber<ValueType>(sub, count_of_on_completed_required, forwarding_on_next{}));
        };

        return create_proxy_subscriber<Type>(std::forward<TSub>(subscriber), count_of_on_completed_required, std::move(on_new_observable));
    };
}
} // namespace rpp::details
