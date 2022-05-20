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
     * \brief emit only those items from an Observable that satisfies a predicate
     *
     * \details The Filter operator applies a provided predicate to each item emitted by the Observable, and returns an Observable that emits only items passed provided predicate
     *
     * Example:
     * \snippet filter.cpp Filter
     *
     * \see https://reactivex.io/documentation/operators/filter.html
     *
     * \tparam Predicate type of predicate used to check emitted itemss
     * \return new specific_observable with the Filter operator as most recent operator.
     * \warning #include <rpp/operators/filter.hpp>
     * \ingroup operators
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
