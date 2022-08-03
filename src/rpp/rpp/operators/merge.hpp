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
struct merge_state_t : public std::enable_shared_from_this<merge_state_t>
{
    auto wrap_under_guard(const auto& callable)
    {
        return [state = shared_from_this(), callable](auto&&...args)
        {
            std::lock_guard lock{state->mutex};
            callable(std::forward<decltype(args)>(args)...);
        };
    }

    auto get_on_completed()
    {
        return [state = shared_from_this()](const constraint::subscriber auto& sub)
        {
            if (state->count_of_on_completed.fetch_sub(1, std::memory_order::acq_rel) == 1)
                sub.on_completed();
        };
    }

    auto get_on_new_observable()
    {
        return  [state = shared_from_this()]<constraint::observable TObs>(const TObs& new_observable, const constraint::subscriber auto& sub)
        {
            using ValueType = utils::extract_observable_type_t<TObs>;

            new_observable.subscribe(combining::create_proxy_subscriber<ValueType>(sub,
                                                                                   state->count_of_on_completed,
                                                                                   state->wrap_under_guard(utils::forwarding_on_next{}),
                                                                                   state->wrap_under_guard(utils::forwarding_on_error{}),
                                                                                   state->get_on_completed()));
        };
    }

    std::atomic_size_t count_of_on_completed{};
private:
    std::mutex         mutex{};
};

template<constraint::decayed_type Type>
struct merge_impl
{
    using ValueType = utils::extract_observable_type_t<Type>;

    template<constraint::subscriber_of_type<ValueType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        const auto state = std::make_shared<merge_state_t>();

        return combining::create_proxy_subscriber<Type>(std::forward<TSub>(subscriber),
                                                        state->count_of_on_completed,
                                                        state->get_on_new_observable(),
                                                        state->wrap_under_guard(utils::forwarding_on_error{}),
                                                        state->get_on_completed());
    }
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> ... TObservables>
auto merge_with_impl(TObservables&&... observables)
{
    return rpp::source::just(std::forward<TObservables>(observables).as_dynamic()...).merge();
}
} // namespace rpp::details
