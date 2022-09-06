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
#include <rpp/utils/functors.hpp>

#include <array>
#include <atomic>
#include <memory>

IMPLEMENTATION_FILE(merge_tag);

namespace rpp::details
{
struct merge_state
{
    std::mutex         mutex{};
    std::atomic_size_t count_of_on_completed_needed{};
};

struct merge_forwarding_on_next
{
    void operator()(auto&&                              value,
                    const constraint::subscriber auto&  sub,
                    const auto& state) const
    {
        std::lock_guard lock{state->mutex};
        sub.on_next(std::forward<decltype(value)>(value));
    }
};

struct merge_on_error
{
    void operator()(const std::exception_ptr& err, const constraint::subscriber auto& sub, const auto& state) const
    {
        std::lock_guard lock{state->mutex};
        sub.on_error(err);
    }
};

struct merge_on_completed
{
    void operator()(const constraint::subscriber auto&  sub,
                    const std::shared_ptr<merge_state>& state) const
    {
        if (state->count_of_on_completed_needed.fetch_sub(1, std::memory_order::acq_rel) == 1)
            sub.on_completed();
    }
};

struct merge_on_next
{
    template<constraint::observable TObs>
    void operator()(const TObs&                         new_observable,
                    const constraint::subscriber auto&  sub,
                    const std::shared_ptr<merge_state>& state) const
    {
        using ValueType = utils::extract_observable_type_t<TObs>;

        state->count_of_on_completed_needed.fetch_add(1, std::memory_order::relaxed);

        new_observable.subscribe(create_subscriber_with_state<ValueType>(sub.get_subscription().make_child(),
                                                                         merge_forwarding_on_next{},
                                                                         merge_on_error{},
                                                                         merge_on_completed{},
                                                                         sub,
                                                                         state));
    }
};

template<constraint::decayed_type Type>
struct merge_impl
{
    using ValueType = utils::extract_observable_type_t<Type>;

    template<constraint::subscriber_of_type<ValueType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<merge_state>();

        state->count_of_on_completed_needed.fetch_add(1, std::memory_order::relaxed);

        auto subscription = subscriber.get_subscription().make_child();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  merge_on_next{},
                                                  merge_on_error{},
                                                  merge_on_completed{},
                                                  std::forward<TSub>(subscriber),
                                                  std::move(state));
    }
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> ... TObservables>
auto merge_with_impl(TObservables&&... observables)
{
    return source::just(std::forward<TObservables>(observables).as_dynamic()...).merge();
}
} // namespace rpp::details
