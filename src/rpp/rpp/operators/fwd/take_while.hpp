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
struct take_while_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, std::predicate<const Type&> Predicate>
auto take_while_impl(Predicate&& predicate);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, take_while_tag>
{
    /**
     * \brief Sends items provided by observable while items are satisfy predicate. When condition becomes false -> sends `on_completed`
     *
     * \marble{take_while,
        {
            source observable                : +--1-2-3-4-5-6-|
            operator "take_while: x => x!=3" : +--1-2-|
        }}
     * \param predicate is predicate used to check items
     * \return new specific_observable with the take_while operator as most recent operator.
     * \warning #include <rpp/operators/take_while.hpp>
     * 
     * \par Example:
     * \snippet take_while.cpp take_while
     *
     * \ingroup conditional_operators
     * \see https://reactivex.io/documentation/operators/takewhile.html
     */
    template<std::predicate<const Type&> Predicate>
    auto take_while(Predicate&& predicate) const& requires is_header_included<take_while_tag, Predicate>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(take_while_impl<Type>(std::forward<Predicate>(predicate)));
    }

    template<std::predicate<const Type&> Predicate>
    auto take_while(Predicate&& predicate) && requires is_header_included<take_while_tag, Predicate>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(take_while_impl<Type>(std::forward<Predicate>(predicate)));
    }
};
} // namespace rpp::details
