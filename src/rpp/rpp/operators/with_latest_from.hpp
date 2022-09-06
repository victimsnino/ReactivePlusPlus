//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once


#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state

#include <rpp/defs.hpp>

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/with_latest_from.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/utilities.hpp>
#include <rpp/utils/functors.hpp>

#include <mutex>
#include <array>
#include <tuple>

IMPLEMENTATION_FILE(with_latest_from_tag);

namespace rpp::details
{
template<typename TSelector, constraint::decayed_type... ValueTypes>
struct with_latest_from_state : early_unsubscribe_state
{
    with_latest_from_state(const TSelector& selector, const composite_subscription& subscription_of_subscriber)
        : early_unsubscribe_state{subscription_of_subscriber}
        , selector(selector) {}

    // RPP_NO_UNIQUE_ADDRESS commented due to MSVC issue for base classes
    /*RPP_NO_UNIQUE_ADDRESS*/ TSelector           selector; 
    std::mutex                                    mutex{};
    std::array<std::mutex, sizeof...(ValueTypes)> mutexes{};
    std::tuple<std::optional<ValueTypes>...>      vals{};
};

template<size_t I>
struct with_latest_from_on_next_inner
{
    template<typename TSelector, constraint::decayed_type... ValueTypes>
    void operator()(auto&&                                                                   value,
                    const constraint::subscriber auto&                                       ,
                    const std::shared_ptr<with_latest_from_state<TSelector, ValueTypes...>>& state) const
    {
        std::lock_guard lock{state->mutexes[I]};
        std::get<I>(state->vals) = std::forward<decltype(value)>(value);
    }
};

using with_latest_from_on_error = merge_on_error;

struct with_latest_from_on_completed_outer
{
    void operator()(const constraint::subscriber auto& sub, const auto& state) const
    {
        state->childs_subscriptions.unsubscribe();
        std::lock_guard lock{state->mutex};
        sub.on_completed();
    }
};

template<size_t I, constraint::observable TObs>
void with_latest_from_subscribe(const auto& state_ptr, const TObs& observable, const auto& subscriber)
{
    using Type = utils::extract_observable_type_t<TObs>;
    observable.subscribe(create_subscriber_with_state<Type>(state_ptr->childs_subscriptions.make_child(),
                                                            with_latest_from_on_next_inner<I>{},
                                                            with_latest_from_on_error{},
                                                            [](const auto&, const auto&) {},
                                                            subscriber,
                                                            state_ptr));
}

template<size_t...I>
void with_latest_from_subscribe_observables(std::index_sequence<I...>,
                                            const auto&              state_ptr,
                                            const auto&              subscriber,
                                            const auto&              observables_tuple)
{
    (with_latest_from_subscribe<I>(state_ptr, std::get<I>(observables_tuple), subscriber), ...);
}



struct with_latest_from_on_next_outer
{
    template<typename T, typename TSelector, constraint::decayed_type... ValueTypes>
    void operator()(T&&                                                                      v,
                    const auto&                                                              sub,
                    const std::shared_ptr<with_latest_from_state<TSelector, ValueTypes...>>& state) const
    {
        using ResultType = utils::decayed_invoke_result_t<TSelector, std::decay_t<T>, ValueTypes...>;

        auto result = std::apply([&](const auto&...current_cached_vals) -> std::optional<ResultType>
                                 {
                                     auto lock = std::apply([](auto&...mutexes)
                                                            {
                                                                return std::scoped_lock{mutexes...};
                                                            },
                                                            state->mutexes);

                                     if ((current_cached_vals.has_value() && ...))
                                         return state->selector(utils::as_const(std::forward<T>(v)),
                                                                utils::as_const(current_cached_vals.value())...);
                                     return std::nullopt;
                                 },
                                 state->vals);

        if (result.has_value())
        {
            std::lock_guard lock{state->mutex};
            sub.on_next(std::move(result.value()));
        }
    }
};

template<constraint::decayed_type Type, typename TSelector, constraint::observable ...TObservables>
struct with_latest_from_impl
{
    using ResultType = utils::decayed_invoke_result_t<
        TSelector, Type, utils::extract_observable_type_t<TObservables>...>;

    RPP_NO_UNIQUE_ADDRESS TSelector                   selector;
    RPP_NO_UNIQUE_ADDRESS std::tuple<TObservables...> observables;

    template<constraint::subscriber_of_type<ResultType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<with_latest_from_state<TSelector, utils::extract_observable_type_t<TObservables>...>>(selector, subscriber.get_subscription());

        with_latest_from_subscribe_observables(std::index_sequence_for<TObservables...>{},
                                               state,
                                               subscriber,
                                               observables);

        auto sub = state->childs_subscriptions.make_child();
        return create_subscriber_with_state<Type>(std::move(sub),
                                                  with_latest_from_on_next_outer{},
                                                  with_latest_from_on_error{},
                                                  with_latest_from_on_completed_outer{},
                                                  std::forward<TSub>(subscriber),
                                                  std::move(state));
    }
};
} // namespace rpp::details
