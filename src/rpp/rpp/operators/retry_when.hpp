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
    template<rpp::constraint::observer     TObserver,
             rpp::constraint::observable   TObservable,
             rpp::constraint::decayed_type TNotifier>
    struct retry_when_state final : public rpp::composite_disposable
    {
        template<typename Observer, typename Observable, typename Notifier>
        retry_when_state(Observer&& observer, Observable&& observable, Notifier&& notifier)
            : observer(std::forward<Observer>(observer))
            , observable(std::forward<Observable>(observable))
            , notifier(std::forward<Notifier>(notifier))
        {
        }

        bool             retrying{};
        std::atomic_bool is_inside_drain{};

        RPP_NO_UNIQUE_ADDRESS TObserver   observer;
        RPP_NO_UNIQUE_ADDRESS TObservable observable;
        RPP_NO_UNIQUE_ADDRESS TNotifier   notifier;
    };

    template<rpp::constraint::observer TObserver, rpp::constraint::observable TObservable, rpp::constraint::decayed_type TNotifier>
    void drain(const std::shared_ptr<retry_when_state<std::decay_t<TObserver>, std::decay_t<TObservable>, std::decay_t<TNotifier>>>& state);

    template<rpp::constraint::observer     TObserver,
             rpp::constraint::observable   TObservable,
             rpp::constraint::decayed_type TNotifier>
    struct retry_when_impl_inner_strategy
    {
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        std::shared_ptr<retry_when_state<TObserver, TObservable, TNotifier>> state;

        template<typename T>
        void on_next(T&&) const
        {
            drain<TObserver, TObservable, TNotifier>(state);
        }

        void on_error(const std::exception_ptr& err) const
        {
            if (!state->is_inside_drain.exchange(false, std::memory_order::seq_cst))
                state->observer.on_error(err);
        }

        void on_completed() const
        {
            if (!state->is_inside_drain.exchange(false, std::memory_order::seq_cst))
                state->observer.on_completed();
        }

        void set_upstream(const disposable_wrapper& d) { state->observer.set_upstream(d); }

        bool is_disposed() const { return state->retrying; }
    };

    template<rpp::constraint::observer     TObserver,
             rpp::constraint::observable   TObservable,
             rpp::constraint::decayed_type TNotifier>
    struct retry_when_impl_strategy
    {
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        std::shared_ptr<retry_when_state<TObserver, TObservable, TNotifier>> state;

        template<typename T>
        void on_next(T&& v) const
        {
            state->observer.on_next(std::forward<T>(v));
        }

        void on_error(const std::exception_ptr& err) const
        {
            std::optional<std::invoke_result_t<TNotifier, std::exception_ptr>> notifier_obs;
            try
            {
                notifier_obs.emplace(state->notifier(err));
            }
            catch (...)
            {
                state->observer.on_error(std::current_exception());
            }
            if (notifier_obs.has_value())
            {
                std::move(notifier_obs).value().subscribe(retry_when_impl_inner_strategy<TObserver, TObservable, TNotifier>{state});
            }
        }

        void on_completed() const
        {
            state->observer.on_completed();
        }

        void set_upstream(const disposable_wrapper& d) { state->add(d); }

        bool is_disposed() const { return state->is_disposed(); }
    };

    template<rpp::constraint::observer TObserver, rpp::constraint::observable TObservable, rpp::constraint::decayed_type TNotifier>
    void drain(const std::shared_ptr<retry_when_state<std::decay_t<TObserver>, std::decay_t<TObservable>, std::decay_t<TNotifier>>>& state)
    {
        while (!state->is_disposed())
        {
            state->clear();
            state->is_inside_drain.store(true, std::memory_order::seq_cst);
            try
            {
                using value_type = rpp::utils::extract_observable_type_t<TObservable>;
                state->observable.subscribe(rpp::observer<value_type, retry_when_impl_strategy<TObserver, TObservable, TNotifier>>{state});

                if (!state->is_inside_drain.exchange(false, std::memory_order::seq_cst))
                    return;
            }
            catch (...)
            {
                state->observer.on_error(std::current_exception());
                return;
            }
        }
    }

    template<rpp::constraint::observable TObservable, rpp::constraint::decayed_type TNotifier>
    struct retry_when_impl_t
    {
        RPP_NO_UNIQUE_ADDRESS TObservable observable;
        RPP_NO_UNIQUE_ADDRESS TNotifier   notifier;

        using value_type = rpp::utils::extract_observable_type_t<TObservable>;

        template<rpp::constraint::observer TObserver>
        void subscribe(TObserver&& observer) const
        {
            const auto d   = disposable_wrapper_impl<retry_when_state<std::decay_t<TObserver>, std::decay_t<TObservable>, std::decay_t<TNotifier>>>::make(std::forward<TObserver>(observer), observable, notifier);
            auto       ptr = d.lock();
            ptr->observer.set_upstream(d.as_weak());

            drain<TObserver, TObservable, TNotifier>(ptr);
        }
    };

    template<rpp::constraint::decayed_type TNotifier>
    struct retry_when_t
    {
        RPP_NO_UNIQUE_ADDRESS TNotifier notifier;

        template<rpp::constraint::observable TObservable>
        auto operator()(TObservable&& observable) const &
        {
            return rpp::observable<utils::extract_observable_type_t<TObservable>, retry_when_impl_t<std::decay_t<TObservable>, std::decay_t<TNotifier>>>{observable, notifier};
        }

        template<rpp::constraint::observable TObservable>
        auto operator()(TObservable&& observable) &&
        {
            return rpp::observable<utils::extract_observable_type_t<TObservable>, retry_when_impl_t<std::decay_t<TObservable>, std::decay_t<TNotifier>>>{observable, std::forward<TNotifier>(notifier)};
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
     * @warning #include <rpp/operators/retry_when.hpp>
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
