//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observers/state_observer.hpp>

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/with_latest_from.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/utilities.hpp>
#include <rpp/utils/functors.hpp>


#include <shared_mutex>
#include <array>

IMPLEMENTATION_FILE(with_latest_from_tag);

namespace rpp::details
{
template<size_t I, constraint::observable TObs>
void with_latest_from_subscribe(const auto& state_ptr, const TObs& observable, const auto& subscriber)
{
    using Type = utils::extract_observable_type_t<TObs>;
    observable.subscribe(create_subscriber_with_state<Type>(subscriber.get_subscription().make_child(),
                                                            subscriber,
                                                            [state_ptr](auto&& v, const auto&)
                                                            {
                                                                std::lock_guard lock{state_ptr->mutexes[I]};
                                                                std::get<I>(state_ptr->vals) = std::forward<decltype(v)>(v);
                                                            },
                                                            forwarding_on_error{},
                                                            [](const auto&){}));
}

template<size_t...I>
void with_latest_from_subscribe_observables(std::index_sequence<I...>,
                                            const auto&              state_ptr,
                                            const auto&              subscriber,
                                            const auto&...           observables)
{
    (with_latest_from_subscribe<I>(state_ptr, observables, subscriber), ...);
}

template<constraint::observable ...TObservables>
struct with_latest_from_state_t
{
    std::array<std::shared_mutex, sizeof...(TObservables)>                       mutexes{};
    std::tuple<std::optional<utils::extract_observable_type_t<TObservables>>...> vals{};

    auto apply_under_lock(const auto& selector)
    {
        auto lock = lock_all();
        return std::apply(selector, vals);
    }

private:
    auto lock_all()
    {
        return std::apply([](auto& ...mutexes) { return std::scoped_lock{ mutexes... }; }, mutexes);
    }
};

template<constraint::decayed_type Type, constraint::observable ...TObservables, std::invocable<Type, utils::extract_observable_type_t<TObservables>...> TSelector>
auto with_latest_from_impl(TSelector&& selector, TObservables&&...observables)
{
    using ResultType = std::invoke_result_t<TSelector, Type, utils::extract_observable_type_t<TObservables>...>;

    return [selector=std::forward<TSelector>(selector), ...observables=std::forward<TObservables>(observables)]<constraint::subscriber_of_type<ResultType> TSub>(TSub&& subscriber)
    {
        auto state = std::make_shared<with_latest_from_state_t<TObservables...>>();

        with_latest_from_subscribe_observables(std::make_index_sequence<sizeof...(TObservables)>{}, state, subscriber, observables...);

        auto on_next = [state, selector](auto&& v, const auto& sub)
        {
            auto result = state->apply_under_lock([&](const auto& ...args) -> std::optional<ResultType>
            {
                if ((args.has_value() && ...))
                    return selector(std::forward<decltype(v)>(v), args.value()...);
                return std::nullopt;
            });

            if (result.has_value())
                sub.on_next(std::move(result.value()));
        };

        return create_subscriber_with_state<Type>(std::forward<TSub>(subscriber),
                                                  std::move(on_next),
                                                  forwarding_on_error{},
                                                  forwarding_on_completed{});
    };
}
} // namespace rpp::details
