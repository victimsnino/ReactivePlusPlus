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
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/utilities.hpp>
#include <rpp/utils/functors.hpp>

#include <mutex>
#include <array>
#include <tuple>

IMPLEMENTATION_FILE(with_latest_from_tag);

namespace rpp::details
{
template<size_t I, constraint::observable TObs>
void with_latest_from_subscribe(const auto& state_ptr, const TObs& observable, const auto& subscriber)
{
    using Type = utils::extract_observable_type_t<TObs>;
    observable.subscribe(create_subscriber_with_state<Type>(subscriber.get_subscription().make_child(),
                                                            [state_ptr](auto&& v, const auto&)
                                                            {
                                                                std::lock_guard lock{state_ptr->mutexes[I]};
                                                                std::get<I>(state_ptr->vals) = std::forward<decltype(v)>(v);
                                                            },
                                                            utils::forwarding_on_error{},
                                                            [](const auto&){},
                                                            subscriber));
}

template<size_t...I>
void with_latest_from_subscribe_observables(std::index_sequence<I...>,
                                            const auto&              state_ptr,
                                            const auto&              subscriber,
                                            const auto&              observables_tuple)
{
    (with_latest_from_subscribe<I>(state_ptr, std::get<I>(observables_tuple), subscriber), ...);
}

template<typename TSelector, constraint::decayed_type ...ValueTypes>
struct with_latest_from_state_t
{
    with_latest_from_state_t(const TSelector& selector) : selector(selector) {}
    
    RPP_NO_UNIQUE_ADDRESS TSelector                                     selector;
    std::array<std::mutex, sizeof...(ValueTypes)> mutexes{};
    std::tuple<std::optional<ValueTypes>...>      vals{};

    auto apply_under_lock(const auto& fn)
    {
        auto lock = lock_all();
        return std::apply(fn, vals);
    }

private:
    auto lock_all()
    {
        return std::apply([](auto& ...mutexes) { return std::scoped_lock{mutexes...}; }, mutexes);
    }
};

template<constraint::decayed_type Type, typename TSelector, constraint::observable ...TObservables>
struct with_latest_from_impl
{
    using ResultType = utils::decayed_invoke_result_t<TSelector, Type, utils::extract_observable_type_t<TObservables>...>;

    RPP_NO_UNIQUE_ADDRESS TSelector                   selector;
    RPP_NO_UNIQUE_ADDRESS std::tuple<TObservables...> observables;

    template<constraint::subscriber_of_type<ResultType> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<with_latest_from_state_t<TSelector, utils::extract_observable_type_t<TObservables>...>>(selector);

        with_latest_from_subscribe_observables(std::index_sequence_for<TObservables...>{}, state, subscriber, observables);

        auto on_next = [state]<typename T>(T&& v, const auto& sub)
        {
            auto result = state->apply_under_lock([&](const auto& ...current_cached_vals) -> std::optional<ResultType>
            {
                if ((current_cached_vals.has_value() && ...))
                    return state->selector(utils::as_const(std::forward<T>(v)), utils::as_const(current_cached_vals.value())...);
                return std::nullopt;
            });

            if (result.has_value())
                sub.on_next(std::move(result.value()));
        };

        auto sub = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(sub),
                                                  std::move(on_next),
                                                  utils::forwarding_on_error{},
                                                  utils::forwarding_on_completed{},
                                                  std::forward<TSub>(subscriber));
    }
};
} // namespace rpp::details
