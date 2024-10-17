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

#include <rpp/observables/fwd.hpp>
#include <rpp/operators/fwd.hpp>

#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/utils.hpp>

namespace rpp::operators::details
{
    template<rpp::constraint::observer TObserver,
             typename TObservable,
             typename TNotifier>
    struct retry_when_state final
    {
        retry_when_state(TObserver&& in_observer, const TObservable& observable, const TNotifier& notifier)
            : observer(std::move(in_observer))
            , observable(observable)
            , notifier(notifier)
        {
            observer.set_upstream(disposable);
        }

        std::atomic_bool is_inside_drain{};

        RPP_NO_UNIQUE_ADDRESS TObserver   observer;
        RPP_NO_UNIQUE_ADDRESS TObservable observable;
        RPP_NO_UNIQUE_ADDRESS TNotifier   notifier;

        rpp::composite_disposable_wrapper disposable = composite_disposable_wrapper::make();
    };

    template<rpp::constraint::observer TObserver, typename TObservable, typename TNotifier>
    void drain(const std::shared_ptr<retry_when_state<TObserver, TObservable, TNotifier>>& state);

    template<rpp::constraint::observer TObserver,
             typename TObservable,
             typename TNotifier>
    struct retry_when_impl_inner_strategy
    {
        static constexpr auto preferred_disposable_mode = rpp::details::observers::disposable_mode::None;

        std::shared_ptr<retry_when_state<TObserver, TObservable, TNotifier>> state;
        mutable bool                                                         locally_disposed{};

        template<typename T>
        void on_next(T&&) const
        {
            locally_disposed = true;
            state->disposable.clear();

            if (state->is_inside_drain.exchange(false, std::memory_order::seq_cst))
                return;

            drain<TObserver, TObservable, TNotifier>(state);
        }

        void on_error(const std::exception_ptr& err) const
        {
            locally_disposed = true;
            state->observer.on_error(err);
        }

        void on_completed() const
        {
            locally_disposed = true;
            state->observer.on_completed();
        }

        void set_upstream(const disposable_wrapper& d) const { state->disposable.add(d); }

        bool is_disposed() const { return locally_disposed || state->disposable.is_disposed(); }
    };

    template<rpp::constraint::observer TObserver,
             typename TObservable,
             typename TNotifier>
    struct retry_when_impl_strategy
    {
        static constexpr auto preferred_disposable_mode = rpp::details::observers::disposable_mode::None;

        std::shared_ptr<retry_when_state<TObserver, TObservable, TNotifier>> state;

        template<typename T>
        void on_next(T&& v) const
        {
            state->observer.on_next(std::forward<T>(v));
        }

        void on_error(const std::exception_ptr& err) const
        {
            try
            {
                state->notifier(err).subscribe(retry_when_impl_inner_strategy<TObserver, TObservable, TNotifier>{state});
            }
            catch (...)
            {
                state->observer.on_error(std::current_exception());
            }
        }

        void on_completed() const
        {
            state->observer.on_completed();
        }

        void set_upstream(const disposable_wrapper& d) { state->disposable.add(d); }

        bool is_disposed() const { return state->disposable.is_disposed(); }
    };

    template<rpp::constraint::observer TObserver, typename TObservable, typename TNotifier>
    void drain(const std::shared_ptr<retry_when_state<TObserver, TObservable, TNotifier>>& state)
    {
        while (!state->disposable.is_disposed())
        {
            state->is_inside_drain.store(true, std::memory_order::seq_cst);
            try
            {
                using value_type = rpp::utils::extract_observer_type_t<TObserver>;
                state->observable.subscribe(rpp::observer<value_type, retry_when_impl_strategy<TObserver, TObservable, TNotifier>>{state});

                if (state->is_inside_drain.exchange(false, std::memory_order::seq_cst))
                    return;
            }
            catch (...)
            {
                state->observer.on_error(std::current_exception());
                return;
            }
        }
    }

    template<rpp::constraint::decayed_type TNotifier>
    struct retry_when_t
    {
        RPP_NO_UNIQUE_ADDRESS TNotifier notifier;

        template<rpp::constraint::decayed_type T>
        struct operator_traits
        {
            using result_type = T;
        };

        template<rpp::details::observables::constraint::disposable_strategy Prev>
        using updated_disposable_strategy = rpp::details::observables::fixed_disposable_strategy_selector<1>;

        template<rpp::constraint::observer TObserver, typename TObservable>
        void subscribe(TObserver&& observer, TObservable&& observable) const
        {
            const auto ptr = std::make_shared<retry_when_state<std::decay_t<TObserver>, std::decay_t<TObservable>, std::decay_t<TNotifier>>>(std::forward<TObserver>(observer), std::forward<TObservable>(observable), notifier);
            drain(ptr);
        }
    };
} // namespace rpp::operators::details

namespace rpp::operators
{
    /**
     * @brief If an error occurs, invoke @p notifier and when returned observable emits a value
     * resubscribe to the source observable. If the notifier throws or returns an error/empty
     * observable, then error/completed emission is forwarded to original subscription.
     *
     * @param notifier callable taking a std::exception_ptr and returning observable notifying when to resubscribe
     *
     * @warning retry_when along with other re-subscribing operators needs to be carefully used with
     * hot observables, as re-subscribing to a hot observable can have unwanted behaviors. For example,
     * a hot observable behind a replay subject can indefinitely yield an error on each re-subscription
     * and using retry_when on it would lead to an infinite execution.
     *
     * @note `#include <rpp/operators/retry_when.hpp>`
     *
     * @par Examples:
     * @snippet retry_when.cpp retry_when delay
     * @snippet retry_when.cpp retry_when
     *
     * @ingroup error_handling_operators
     * @see https://reactivex.io/documentation/operators/retry.html
     */
    template<typename TNotifier>
        requires rpp::constraint::observable<std::invoke_result_t<TNotifier, std::exception_ptr>>
    auto retry_when(TNotifier&& notifier)
    {
        return details::retry_when_t<std::decay_t<TNotifier>>{std::forward<TNotifier>(notifier)};
    }
} // namespace rpp::operators
