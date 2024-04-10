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
    template<rpp::constraint::observer TObserver, rpp::constraint::decayed_type Selector>
    struct on_error_resume_next_observer_strategy
    {
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        RPP_NO_UNIQUE_ADDRESS mutable TObserver observer;
        RPP_NO_UNIQUE_ADDRESS Selector          selector;
        // Manually control disposable to ensure observer is not used after move in on_error emission
        mutable rpp::composite_disposable_wrapper disposable = composite_disposable_wrapper::make();

        RPP_CALL_DURING_CONSTRUCTION(
            observer.set_upstream(disposable););

        template<typename T>
        void on_next(T&& v) const
        {
            observer.on_next(std::forward<T>(v));
        }

        void on_error(const std::exception_ptr& err) const
        {
            disposable.dispose();
            selector(err).subscribe(std::move(observer));
        }

        void on_completed() const
        {
            disposable.dispose();
            observer.on_completed();
        }

        void set_upstream(const disposable_wrapper& d)
        {
            disposable.add(d);
        }

        bool is_disposed() const { return disposable.is_disposed(); }
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
        using updated_disposable_strategy = rpp::details::observables::atomic_dynamic_disposable_strategy_selector<1>;
    };
} // namespace rpp::operators::details

namespace rpp::operators
{
    /**
     * @brief If an error occurs, take the result from the Selector and subscribe to that instead.
     *
     * @marble on_error_resume_next
       {
           source observable                                  : +-1-x
           operator "on_error_resume_next: () => obs(-2-3-|)" : +-1-2-3-|
       }
     *
     * @param selector callable taking a std::exception_ptr and returning observable to continue on
     *
     * @warning #include <rpp/operators/on_error_resume_next.hpp>
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
