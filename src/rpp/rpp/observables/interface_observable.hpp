//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/fwd.hpp>
#include <rpp/observers/anonymous_observer.hpp>
#include <rpp/disposables/composite_disposable.hpp>

namespace rpp
{
template<constraint::decayed_type Type>
struct interface_observable
{
    virtual ~interface_observable() = default;

    composite_disposable subscribe(const interface_observer<Type>& observer) const
    {
        return subscribe_impl(observer);
    }

    template<constraint::on_next_fn<Type> TOnNext      = utils::empty_function_t<Type>,
             constraint::on_error_fn      TOnError     = utils::rethrow_error_t,
             constraint::on_completed_fn  TOnCompleted = utils::empty_function_t<>>
    composite_disposable subscribe(TOnNext&& on_next = {}, TOnError&& on_error = {}, TOnCompleted&& on_completed = {}) const
    {
        return subscribe(anonymous_observer<Type,
                                            std::decay_t<TOnNext>,
                                            std::decay_t<TOnError>,
                                            std::decay_t<TOnCompleted>>{
                             std::forward<TOnNext>(on_next),
                             std::forward<TOnError>(on_error),
                             std::forward<TOnCompleted>(on_completed)});
    }

    
    template<constraint::on_next_fn<Type> TOnNext      = utils::empty_function_t<Type>,
             constraint::on_completed_fn  TOnCompleted = utils::empty_function_t<>>
    composite_disposable subscribe(TOnNext&& on_next, TOnCompleted&& on_completed) const
    {
        return subscribe(std::forward<TOnNext>(on_next), utils::rethrow_error_t{}, std::forward<TOnCompleted>(on_completed));
    }

protected:
    virtual composite_disposable subscribe_impl(const interface_observer<Type>& observer) const = 0;
};
} // namespace rpp