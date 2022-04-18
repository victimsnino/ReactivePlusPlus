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
#include <rpp/sources/just.h>

#include <atomic>
#include <memory>

IMPLEMENTATION_FILE(merge_tag);

namespace rpp::operators
{
template<constraint::observable ...TObservables>
auto merge(TObservables&&...observables) requires details::is_header_included<details::merge_tag, TObservables...>
{
    return [...observables = std::forward<TObservables>(observables)]<constraint::observable TObservable>(TObservable && observable)
    {
        return std::forward<TObservable>(observable).merge(observables...);
    };
}
} // namespace rpp::operators

namespace rpp::details
{
struct state_t
{
    std::atomic_size_t count_of_on_completed{};
    std::mutex         mutex{};
};

template<constraint::decayed_type Type>
auto create_proxy_subscriber(constraint::subscriber auto&&   subscriber,
                             const std::shared_ptr<state_t>& state,
                             auto&&                          on_next,
                             auto&&                          on_error,
                             auto&&                          on_completed)
{
    ++(state->count_of_on_completed);

    auto subscription = subscriber.get_subscription();
    auto result       = create_subscriber_with_state<Type>(std::forward<decltype(subscriber)>(subscriber),
                                                           std::forward<decltype(on_next)>(on_next),
                                                           std::forward<decltype(on_error)>(on_error),
                                                           std::forward<decltype(on_completed)>(on_completed));

    subscription.add(result.get_subscription());
    return result;
}

template<constraint::decayed_type Type, typename SpecificObservable>
auto member_overload<Type, SpecificObservable, merge_tag>::merge_impl()
{
    using ValueType = utils::extract_observable_type_t<Type>;

    return []<constraint::subscriber_of_type<ValueType> TSub>(TSub&& subscriber)
    {
        auto state = std::make_shared<state_t>();

        auto wrap_under_guard = [state](const auto& callable)
        {
            return [state, callable](auto&&...args)
            {
                std::lock_guard lock{ state->mutex };
                callable(std::forward<decltype(args)>(args)...);
            };
        };

        auto on_completed = [=](const constraint::subscriber auto& sub)
        {
            if (--(state->count_of_on_completed) == 0)
                sub.on_completed();
        };

        auto on_new_observable = [=]<constraint::observable TObs>(TObs&& new_observable,
                                                                  const constraint::subscriber auto& sub)
        {
            std::forward<TObs>(new_observable).subscribe(create_proxy_subscriber<ValueType>(sub,
                                                                                            state,
                                                                                            wrap_under_guard(forwarding_on_next{}),
                                                                                            wrap_under_guard(forwarding_on_error{}),
                                                                                            on_completed));
        };

        return create_proxy_subscriber<Type>(std::forward<TSub>(subscriber),
                                             state,
                                             std::move(on_new_observable),
                                             wrap_under_guard(forwarding_on_error{}),
                                             on_completed);
    };
}

template<constraint::decayed_type Type, typename SpecificObservable>
template<constraint::observable_of_type<Type> ... TObservables>
auto member_overload<Type, SpecificObservable, merge_tag>::merge_with_impl(TObservables&&... observables) requires (sizeof...(TObservables) >= 1)
{
    return [...observables = std::forward<TObservables>(observables)]<constraint::observable TObs>(TObs&& obs)
    {
        return rpp::source::just(std::forward<TObs>(obs).as_dynamic(), std::move(observables).as_dynamic()...).merge();
    };
}
} // namespace rpp::details
