//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//                             TC Wang 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/constraints.hpp>
#include <rpp/observables/details/member_overload.hpp>

namespace rpp::details
{
struct take_until_tag;
}

namespace rpp::details
{

template<constraint::decayed_type Type, constraint::observable TTriggerObservable>
struct take_until_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, take_until_tag>
{

   /**
    * \brief Discard any items emitted by an Observable after a second Observable emits an item or terminates.
    * \warning The take_until subscribes and begins mirroring the source Observable. It also monitors a second Observable that you provide. If this second Observable emits an item or sends a on_error/on_completed notification, the Observable returned by take_until stops mirroring the source Observable and terminates.
    *
    * \marble take_until
    {
        source observable        : +-1--2--3--|
        source until_observable  : +--s--s----|
        operator "take_until"    : +-1-|
    }
    *
    * \details Actually this operator just subscribes on 2 observables and completes original when `until_observable` emits any value
    *
    * \param until_observable is the observables that stops the source observable from sending values when it emits one value or sends a on_error/on_completed event.
    * \return new specific_observable with the take_until operator as most recent operator.
    * \warning #include <rpp/operators/take_until.hpp>
    *
    * \par Examples
    * \snippet take_until.cpp take_until
    * \snippet take_until.cpp terminate
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to store internal state
    * - <b>OnNext for original observable</b>
    *    - Just forward original on_next
    * - <b>OnError for original observable</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted for original observable</b>
    *    - Just forwards original on_completed
    * - <b>OnNext for until observable</b>
    *    - Emits on_completed
    * - <b>OnError for until observable</b>
    *    - Just forwards on_error
    * - <b>OnCompleted for until observable</b>
    *    - Just forwards on_completed
    *
    * \ingroup conditional_operators
    * \see https://reactivex.io/documentation/operators/takeuntil.html
    */
    template<constraint::observable TTriggerObservable>
    auto take_until(TTriggerObservable&& until_observable) const& requires is_header_included<take_until_tag, TTriggerObservable>
    {
        return cast_this()->template lift<Type>(take_until_impl<Type, std::decay_t<TTriggerObservable>>{std::forward<TTriggerObservable>(until_observable)});
    }

    template<constraint::observable TTriggerObservable>
    auto take_until(TTriggerObservable&& until_observable) && requires is_header_included<take_until_tag, TTriggerObservable>
    {
        return move_this().template lift<Type>(take_until_impl<Type, std::decay_t<TTriggerObservable>>{std::forward<TTriggerObservable>(until_observable)});
    }

private:
    const SpecificObservable* cast_this() const
    {
        return static_cast<const SpecificObservable*>(this);
    }

    SpecificObservable&& move_this()
    {
        return std::move(*static_cast<SpecificObservable*>(this));
    }
};

} // namespace rpp::details
