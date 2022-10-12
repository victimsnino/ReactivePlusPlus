//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                    TC Wang 2022 - present.
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
struct last_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type>
struct last_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, last_tag>
{
    /**
     * \brief Emit only the last item provided before on_completed.
     *
     * \marble last
         {
             source observable   : +--1--2--3--|
             operator "last"     : +--3-|
         }
     *
     * \return new specific_observable with the last operator as most recent operator.
     * \warning #include <rpp/operators/last.hpp>
     *
     * \par Example:
     * \snippet last.cpp last
     * \snippet last.cpp last empty
     *
     * \ingroup filtering_operators
     * \see https://reactivex.io/documentation/operators/last.html
     */
    auto last() const & requires is_header_included<last_tag, Type>
    {
        return cast_this()->template lift<Type>(last_impl<Type>{});
    }

    auto last() && requires is_header_included<last_tag, Type>
    {
        return move_this().template lift<Type>(last_impl<Type>{});
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
