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

#include <rpp/observables/details/member_overload.hpp>
#include <rpp/observers/fwd.hpp>
#include <rpp/observers/specific_observer.hpp>

namespace rpp::details
{
struct do_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observer_of_type<Type> TObs>
struct do_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, do_tag>
{
    /**
     * \brief Register an observer to be called when observable provides any events (on_next/on_error/on_completed)
     *
     * \note Callbacks from `tap` would be invoked BEFORE subscribed subscriber
     *
     * \param observer - observer which would accept callbacks
     *
     * \return new specific_observable with the tap operator as most recent operator.
     * \warning #include <rpp/operators/do.hpp>
     * 
     * \par Example
     * \snippet do.cpp tap
	 *
     * \ingroup utility_operators
     * \see https://reactivex.io/documentation/operators/do.html
     */
    template<constraint::observer_of_type<Type> TObs>
    auto tap(TObs&& observer) const& requires is_header_included <do_tag, TObs>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(do_impl<Type, std::decay_t<TObs>>{std::forward< TObs>(observer)});
    }

    template<constraint::observer_of_type<Type> TObs>
    auto tap(TObs&& observer) && requires is_header_included <do_tag, TObs>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(do_impl<Type, std::decay_t<TObs>>{std::forward< TObs>(observer)});
    }

    /**
     * \brief Register an list of actions to be called when observable provides any events (on_next/on_error/on_completed)
     *
     * \note Callbacks from `tap` would be invoked BEFORE subscribed subscriber
     *
     * \param on_next - action over new emitted item
     * \param on_error - action over std::exception_ptr in case of any error
     * \param on_completed - action in case of completion
     *
     * \return new specific_observable with the tap operator as most recent operator.
     * \warning #include <rpp/operators/do.hpp>
     * 
     * \par Example
     * \snippet do.cpp tap
	 *
     * \ingroup utility_operators
     * \see https://reactivex.io/documentation/operators/do.html
     */
    template<constraint::on_next_fn<Type> OnNextFn, constraint::on_error_fn OnErrorFn = utils::empty_function_t<std::exception_ptr>, constraint::on_completed_fn OnCompletedFn = utils::empty_function_t<>>
    auto tap(OnNextFn&& on_next, OnErrorFn&& on_error = OnErrorFn{}, OnCompletedFn&& on_completed = OnCompletedFn{}) const& requires is_header_included <do_tag, OnNextFn, OnErrorFn, OnCompletedFn>
    {
        using TObserver = specific_observer_with_decayed_args<Type, OnNextFn, OnErrorFn, OnCompletedFn>;
        return static_cast<const SpecificObservable*>(this)->tap(TObserver{std::forward<OnNextFn>(on_next),
                                                                           std::forward<OnErrorFn>(on_error),
                                                                           std::forward<OnCompletedFn>(on_completed)});
    }

    template<constraint::on_next_fn<Type> OnNextFn, constraint::on_error_fn OnErrorFn = utils::empty_function_t<std::exception_ptr>, constraint::on_completed_fn OnCompletedFn = utils::empty_function_t<>>
    auto tap(OnNextFn&& on_next, OnErrorFn&& on_error = OnErrorFn{}, OnCompletedFn&& on_completed = OnCompletedFn{}) && requires is_header_included <do_tag, OnNextFn, OnErrorFn, OnCompletedFn>
    {
        using TObserver = specific_observer_with_decayed_args<Type, OnNextFn, OnErrorFn, OnCompletedFn>;
        return std::move(*static_cast<SpecificObservable*>(this)).tap(TObserver{std::forward<OnNextFn>(on_next),
                                                                                std::forward<OnErrorFn>(on_error),
                                                                                std::forward<OnCompletedFn>(on_completed)});
    }
};
} // namespace rpp::details
