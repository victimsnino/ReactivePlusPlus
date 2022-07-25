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

namespace rpp::details
{
struct distinct_until_changed_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type>
struct distinct_until_changed_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, distinct_until_changed_tag>
{
    /**
     * \brief Suppress consecutive duplicates of emissions from original observable
     * 
     * \marble distinct_until_changed
        {
            source observable       : +--1-1-2-2-3-2-1-|
            operator "distinct_until_changed" : +--1---2---3-2-1-|
        }
     *
     * \return new specific_observable with the distinct_until_changed operator as most recent operator.
     * \warning #include <rpp/operators/distinct_until_changed.hpp>
     * 
     * \par Example
     * \snippet distinct_until_changed.cpp distinct_until_changed
	 *
     * \ingroup filtering_operators
     * \see https://reactivex.io/documentation/operators/distinct.html
     */
    template<typename ...Args>
    auto distinct_until_changed() const & requires is_header_included<distinct_until_changed_tag, Args...>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(distinct_until_changed_impl<Type>{});
    }

    template<typename ...Args>
    auto distinct_until_changed() && requires is_header_included<distinct_until_changed_tag, Args...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(distinct_until_changed_impl<Type>{});
    }
};
} // namespace rpp::details
