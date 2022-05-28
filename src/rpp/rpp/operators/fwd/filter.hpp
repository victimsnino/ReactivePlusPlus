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
struct filter_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, std::predicate<const Type&> Predicate>
auto filter_impl(Predicate&& predicate);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, filter_tag>
{
    /**
     * \brief Emit only those items from an Observable that satisfies a provided predicate
     * 
     * \marble{filter,
        {
            source observable            : +--1-2-3-4-|
            operator "filter: x=>x%2==0" : +----2---4-|
        }}
     * 
     * \tparam Predicate type of predicate used to check emitted items. true -> items satisfies condition, false -> not
     * \return new specific_observable with the Filter operator as most recent operator.
     * \warning #include <rpp/operators/filter.hpp>
     * 
     * Example:
     * \snippet filter.cpp Filter
     *
     * \ingroup filtering_operators
     * \see https://reactivex.io/documentation/operators/filter.html
     */
    template<std::predicate<const Type&> Predicate>
    auto filter(Predicate&& predicate) const& requires is_header_included<filter_tag, Predicate>
    {
        return static_cast<const SpecificObservable*>(this)->template lift <Type>(filter_impl<Type>(std::forward<Predicate>(predicate)));
    }

    template<std::predicate<const Type&> Predicate>
    auto filter(Predicate&& predicate) && requires is_header_included<filter_tag, Predicate>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(filter_impl<Type>(std::forward<Predicate>(predicate)));
    }
};
} // namespace rpp::details
