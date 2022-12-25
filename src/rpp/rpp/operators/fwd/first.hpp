//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                            TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/observables/details/member_overload.hpp>

namespace rpp::details
{
struct first_tag;
}

namespace rpp::details
{

template<constraint::decayed_type Type>
struct first_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, first_tag>
{
   /**
    * \brief emit only the first item.
    *
    * \marble first
        {
            source observable   : +--1--2--3--|
            operator "first"    : +--1|
        }
    *
    * \details Actually this operator is `take(1)` with exception during `on_completed` if no any emision happens. So, it just forwards first obtained emission and emits on_completed immediately
    * \throws rpp::utils::not_enough_emissions in case of on_completed obtained without any emissions
    *
    * \return new specific_observable with the first operator as most recent operator.
    * \warning #include <rpp/operators/first.hpp>
    *
    * \par Example:
    * \snippet first.cpp first
    * \snippet first.cpp first_empty
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to keep internal state
    * - <b>OnNext</b>
    *    - Just forwards 1 emission and emit on_completed
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - throws exception if no any emission before
    *
    * \ingroup filtering_operators
    * \see https://reactivex.io/documentation/operators/first.html
    */
    auto first() const & requires is_header_included<first_tag, Type>
    {
        return cast_this()->template lift<Type>(first_impl<Type>{});
    }

    auto first() && requires is_header_included<first_tag, Type>
    {
        return move_this().template lift<Type>(first_impl<Type>{});
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
