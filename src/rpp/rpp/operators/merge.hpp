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

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/merge.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/observers/state_observer.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/operators/details/combining_utils.hpp>

#include <atomic>
#include <memory>

IMPLEMENTATION_FILE(merge_tag);

namespace rpp::details
{
struct merge_state_t
{
    std::atomic_size_t count_of_on_completed{};
    std::mutex         mutex{};
};

template<constraint::decayed_type Type>
auto merge_impl()
{
    using ValueType = utils::extract_observable_type_t<Type>;

    return []<constraint::subscriber_of_type<ValueType> TSub>(TSub&& subscriber)
    {
        auto state = std::make_shared<merge_state_t>();
        auto count_of_on_completed = std::shared_ptr{ state, &state->count_of_on_completed};

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
            if (--(*count_of_on_completed) == 0)
                sub.on_completed();
        };

        auto on_new_observable = [=]<constraint::observable TObs>(TObs&& new_observable,
                                                                  const constraint::subscriber auto& sub)
        {
            std::forward<TObs>(new_observable).subscribe(combining::create_proxy_subscriber<ValueType>(sub,
                                                                                                       count_of_on_completed,
                                                                                                       wrap_under_guard(forwarding_on_next{}),
                                                                                                       wrap_under_guard(forwarding_on_error{}),
                                                                                                       on_completed));
        };

        return combining::create_proxy_subscriber<Type>(std::forward<TSub>(subscriber),
                                                        count_of_on_completed,
                                                        std::move(on_new_observable),
                                                        wrap_under_guard(forwarding_on_error{}),
                                                        on_completed);
    };
}

template<constraint::decayed_type Type, constraint::observable_of_type<Type> ... TObservables>
auto merge_with_impl(TObservables&&... observables) requires (sizeof...(TObservables) >= 1)
{
    return [...observables = std::forward<TObservables>(observables)]<constraint::observable TObs>(TObs&& obs)
    {
        return rpp::source::just(std::forward<TObs>(obs).as_dynamic(), std::move(observables).as_dynamic()...).merge();
    };
}
} // namespace rpp::details
