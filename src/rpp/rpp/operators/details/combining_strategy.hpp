//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/operators/fwd.hpp>

#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/schedulers/current_thread.hpp>
#include <rpp/utils/utils.hpp>

#include <memory>

namespace rpp::operators::details
{
    template<rpp::constraint::observer Observer>
    class combining_state : public rpp::details::enable_wrapper_from_this<combining_state<Observer>>
        , public rpp::details::base_disposable
    {
    public:
        explicit combining_state(Observer&& observer, size_t on_completed_needed)
            : m_observer_with_mutex{std::move(observer)}
            , m_on_completed_needed{on_completed_needed}
        {
        }

        rpp::utils::pointer_under_lock<Observer> get_observer_under_lock() { return m_observer_with_mutex; }

        bool decrement_on_completed()
        {
            // just need atomicity, not guarding anything
            return m_on_completed_needed.fetch_sub(1, std::memory_order::seq_cst) == 1;
        }

    private:
        rpp::utils::value_with_mutex<Observer> m_observer_with_mutex{};

        std::atomic_size_t m_on_completed_needed;
    };

    template<typename TState>
    struct combining_observer_strategy
    {
        std::shared_ptr<TState> state{};

        void set_upstream(const rpp::disposable_wrapper& d) const
        {
            state->get_observer_under_lock()->set_upstream(d);
        }

        bool is_disposed() const
        {
            return state->is_disposed();
        }

        void on_error(const std::exception_ptr& err) const
        {
            state->get_observer_under_lock()->on_error(err);
        }

        void on_completed() const
        {
            if (state->decrement_on_completed())
                state->get_observer_under_lock()->on_completed();
        }
    };

    template<template<typename...> typename TState, template<auto, typename...> typename TStrategy, typename TSelector, rpp::constraint::observable... TObservables>
    struct combining_operator_t
    {
        RPP_NO_UNIQUE_ADDRESS rpp::utils::tuple<TObservables...> observables;
        RPP_NO_UNIQUE_ADDRESS TSelector                          selector;

        template<rpp::constraint::decayed_type T>
        struct operator_traits
        {
            static_assert(std::invocable<TSelector, T, rpp::utils::extract_observable_type_t<TObservables>...>, "Selector is not callable with passed T type");

            using result_type = std::invoke_result_t<TSelector, T, rpp::utils::extract_observable_type_t<TObservables>...>;

            constexpr static bool own_current_queue = true;
        };

        template<rpp::details::observables::constraint::disposable_strategy Prev>
        using updated_disposable_strategy = ::rpp::details::observables::default_disposable_strategy_selector; // TODO: sum of Prev + TObservables

        template<rpp::constraint::decayed_type Type, rpp::constraint::observer Observer>
        auto lift(Observer&& observer) const
        {
            return observables.apply(&subscribe_impl<Type, Observer>, std::forward<Observer>(observer), selector);
        }

    private:
        template<rpp::constraint::decayed_type Type, rpp::constraint::observer Observer>
        static auto subscribe_impl(Observer&& observer, const TSelector& selector, const TObservables&... observables)
        {
            using State = TState<Observer, TSelector, Type, rpp::utils::extract_observable_type_t<TObservables>...>;

            const auto d     = rpp::disposable_wrapper_impl<State>::make(std::forward<Observer>(observer), selector);
            auto       state = d.lock();
            state->get_observer_under_lock()->set_upstream(d.as_weak());

            subscribe<std::decay_t<Type>>(state, std::index_sequence_for<TObservables...>{}, observables...);

            return rpp::observer<Type, TStrategy<0, std::decay_t<Observer>, TSelector, Type, rpp::utils::extract_observable_type_t<TObservables>...>>{std::move(state)};
        }

        template<typename ExpectedValue, rpp::constraint::observer Observer, size_t... I>
        static void subscribe(const std::shared_ptr<TState<Observer, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>>& state, std::index_sequence<I...>, const TObservables&... observables)
        {
            (..., observables.subscribe(rpp::observer<rpp::utils::extract_observable_type_t<TObservables>, TStrategy<I + 1, Observer, TSelector, ExpectedValue, rpp::utils::extract_observable_type_t<TObservables>...>>{state}));
        }
    };
} // namespace rpp::operators::details
