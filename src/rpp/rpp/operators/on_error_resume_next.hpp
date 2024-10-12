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

namespace rpp::operators::details
{
    template<rpp::constraint::observer TObserver>
    struct on_error_resume_next_inner_observer_strategy
    {
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        std::shared_ptr<TObserver> observer;

        template<typename T>
        void on_next(T&& v) const
        {
            observer->on_next(std::forward<T>(v));
        }

        void on_error(const std::exception_ptr& err) const
        {
            observer->on_error(err);
        }

        void on_completed() const
        {
            observer->on_completed();
        }

        void set_upstream(const disposable_wrapper& d) { observer->set_upstream(d); }

        bool is_disposed() const { return observer->is_disposed(); }
    };


    template<rpp::constraint::observer TObserver, rpp::constraint::decayed_type Selector>
    struct on_error_resume_next_observer_strategy
    {
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        on_error_resume_next_observer_strategy(TObserver&& observer, const Selector& selector)
            : state{std::make_shared<TObserver>(std::move(observer))}
            , selector{selector}
        {
        }

        std::shared_ptr<TObserver>     state;
        RPP_NO_UNIQUE_ADDRESS Selector selector;

        template<typename T>
        void on_next(T&& v) const
        {
            state->on_next(std::forward<T>(v));
        }

        void on_error(const std::exception_ptr& err) const
        {
            try
            {
                selector(err).subscribe(on_error_resume_next_inner_observer_strategy<TObserver>{state});
            }
            catch (...)
            {
                state->on_error(std::current_exception());
            }
        }

        void on_completed() const
        {
            state->on_completed();
        }

        void set_upstream(const disposable_wrapper& d) const
        {
            state->set_upstream(d);
        }

        bool is_disposed() const { return state->is_disposed(); }
    };

    template<rpp::constraint::decayed_type Selector>
    struct on_error_resume_next_t : lift_operator<on_error_resume_next_t<Selector>, Selector>
    {
        using lift_operator<on_error_resume_next_t<Selector>, Selector>::lift_operator;

        template<rpp::constraint::decayed_type T>
        struct operator_traits
        {
            using selector_observable_result_type =
                rpp::utils::extract_observable_type_t<std::invoke_result_t<Selector, std::exception_ptr>>;

            static_assert(
                rpp::constraint::decayed_same_as<selector_observable_result_type, T>,
                "Selector observable result type is not the same as T");

            using result_type = T;

            template<rpp::constraint::observer_of_type<result_type> TObserver>
            using observer_strategy = on_error_resume_next_observer_strategy<TObserver, Selector>;
        };

        template<rpp::details::observables::constraint::disposable_strategy Prev>
        using updated_disposable_strategy = rpp::details::observables::default_disposable_strategy_selector;
    };
} // namespace rpp::operators::details

namespace rpp::operators
{
    /**
     * @brief If an error occurs, take the result from the Selector and subscribe to that instead.
     *
     * @marble on_error_resume_next
       {
           source observable                                  : +-1-#
           operator "on_error_resume_next: () => obs(-2-3-|)" : +-1-2-3-|
       }
     *
     * @param selector callable taking a std::exception_ptr and returning observable to continue on
     *
     * @note `#include <rpp/operators/on_error_resume_next.hpp>`
     *
     * @ingroup error_handling_operators
     * @see https://reactivex.io/documentation/operators/catch.html
     */
    template<typename Selector>
        requires rpp::constraint::observable<std::invoke_result_t<Selector, std::exception_ptr>>
    auto on_error_resume_next(Selector&& selector)
    {
        return details::on_error_resume_next_t<std::decay_t<Selector>>{std::forward<Selector>(selector)};
    }
} // namespace rpp::operators
