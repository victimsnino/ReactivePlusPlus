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
             rpp::constraint::decayed_type Notifier>
    struct retry_when_impl_inner_strategy
    {
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        RPP_NO_UNIQUE_ADDRESS mutable TObserver observer;
        RPP_NO_UNIQUE_ADDRESS TObservable       observable;
        RPP_NO_UNIQUE_ADDRESS Notifier          notifier;

        template<typename T>
        void on_next(T&&) const
        {
            observable.subscribe(std::move(observer));
        }

        void on_error(const std::exception_ptr& err) const
        {
            observer.on_error(err);
        }

        void on_completed() const
        {
            observer.on_completed();
        }

        void set_upstream(const disposable_wrapper& d) { observer.set_upstream(d); }

        bool is_disposed() const { return observer.is_disposed(); }
    };

    template<rpp::constraint::observer     TObserver,
             rpp::constraint::observable   TObservable,
             rpp::constraint::decayed_type Notifier>
    struct retry_when_impl_strategy
    {
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        RPP_NO_UNIQUE_ADDRESS mutable TObserver   observer;
        RPP_NO_UNIQUE_ADDRESS mutable TObservable observable;
        RPP_NO_UNIQUE_ADDRESS mutable Notifier    notifier;

        template<typename T>
        void on_next(T&& v) const
        {
            observer.on_next(std::forward<T>(v));
        }

        void on_error(const std::exception_ptr& err) const
        {
            std::optional<std::invoke_result_t<Notifier, std::exception_ptr>> notifier_obs;
            try
            {
                notifier_obs.emplace(notifier(err));
            }
            catch (...)
            {
                observer.on_error(std::current_exception());
            }
            if (notifier_obs.has_value())
            {
                std::move(notifier_obs).value().subscribe(retry_when_impl_inner_strategy<TObserver, TObservable, Notifier>{std::move(observer), std::move(observable), std::move(notifier)});
            }
        }

        void on_completed() const
        {
            observer.on_completed();
        }

        void set_upstream(const disposable_wrapper& d) { observer.set_upstream(d); }

        bool is_disposed() const { return observer.is_disposed(); }
    };

    template<rpp::constraint::observable TObservable, rpp::constraint::decayed_type Notifier>
    struct retry_when_impl_t : lift_operator<retry_when_impl_t<TObservable, Notifier>, TObservable, Notifier>
    {
        using lift_operator<retry_when_impl_t<TObservable, Notifier>, TObservable, Notifier>::lift_operator;

        template<rpp::constraint::decayed_type T>
        struct operator_traits
        {
            using result_type = T;

            template<rpp::constraint::observer_of_type<result_type> TObserver>
            using observer_strategy = retry_when_impl_strategy<TObserver, TObservable, Notifier>;
        };

        template<rpp::details::observables::constraint::disposable_strategy Prev>
        using updated_disposable_strategy = Prev;
    };

    template<rpp::constraint::decayed_type Notifier>
    struct retry_when_t
    {
        RPP_NO_UNIQUE_ADDRESS Notifier notifier;

        template<rpp::constraint::observable TObservable>
        auto operator()(TObservable&& observable) const &
        {
            return std::forward<TObservable>(observable)
                 | retry_when_impl_t<std::decay_t<TObservable>, std::decay_t<Notifier>>{observable, notifier};
        }

        template<rpp::constraint::observable TObservable>
        auto operator()(TObservable&& observable) &&
        {
            return std::forward<TObservable>(observable)
                 | retry_when_impl_t<std::decay_t<TObservable>, std::decay_t<Notifier>>{observable, std::forward<Notifier>(notifier)};
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
    template<typename Notifier>
        requires rpp::constraint::observable<std::invoke_result_t<Notifier, std::exception_ptr>>
    auto retry_when(Notifier&& notifier)
    {
        return details::retry_when_t<std::decay_t<Notifier>>{std::forward<Notifier>(notifier)};
    }
} // namespace rpp::operators
