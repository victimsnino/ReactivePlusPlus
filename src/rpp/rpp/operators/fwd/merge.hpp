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

#include <rpp/observables/constraints.hpp>
#include <rpp/observables/details/member_overload.hpp>
#include <rpp/observables/type_traits.hpp>

namespace rpp::details
{
struct merge_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type>
auto merge_impl();

template<constraint::decayed_type Type, constraint::observable_of_type<Type> ...TObservables>
auto merge_with_impl(TObservables&& ...observables) requires (sizeof...(TObservables) >= 1);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, merge_tag>
{
    /**
    * \brief Converts observable of observables of items into observable of items via merging emissions.
    *
    * \warning According to observable contract (https://reactivex.io/documentation/contract.html) emissions from any observable should be serialized, so, resulting observable uses mutex to satisfy this requirement
    * 
    * \marble merge
        {
            source observable                : 
            {   
                +--1-2-3-|
                .....+4--6-|
            }
            operator "merge" : +--1-243-6-|
        }
    *
    * \return new specific_observable with the merge operator as most recent operator.
    * \warning #include <rpp/operators/merge.hpp>
    * 
    * \par Example:
    * \snippet merge.cpp merge
    * 
    * \ingroup combining_operators
    * \see https://reactivex.io/documentation/operators/merge.html
    */
    template<typename ...Args>
    auto merge() const& requires (is_header_included<merge_tag, Args...> && rpp::constraint::observable<Type>)
    {
        return static_cast<const SpecificObservable*>(this)->template lift<utils::extract_observable_type_t<Type>>(merge_impl<Type>());
    }

    template<typename ...Args>
    auto merge() && requires (is_header_included<merge_tag, Args...>&& rpp::constraint::observable<Type>)
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<utils::extract_observable_type_t<Type>>(merge_impl<Type>());
    }

    /**
    * \brief Combines submissions from current observable with other observables into one
    *
    * \warning According to observable contract (https://reactivex.io/documentation/contract.html) emissions from any observable should be serialized, so, resulting observable uses mutex to satisfy this requirement
    *
    * \marble merge_with
        {
            source original_observable: +--1-2-3-|
            source second: +-----4--6-|
            operator "merge_with" : +--1-243-6-|
        }
    * \param observables are observables whose emissions would be merged with current observable
    * \return new specific_observable with the merge operator as most recent operator.
    * \warning #include <rpp/operators/merge.hpp>
    * 
    * \par Example:
    * \snippet merge.cpp merge_with
    *
    * \ingroup combining_operators
    * \see https://reactivex.io/documentation/operators/merge.html
    */
    template<constraint::observable_of_type<Type> ...TObservables>
    auto merge_with(TObservables&&... observables) const& requires (is_header_included<merge_tag, TObservables...>&& sizeof...(TObservables) >= 1)
    {
        return static_cast<const SpecificObservable*>(this)->op(merge_with_impl<Type>(std::forward<TObservables>(observables)...));
    }

    template<constraint::observable_of_type<Type> ...TObservables>
    auto merge_with(TObservables&&... observables) && requires (is_header_included<merge_tag, TObservables...> && sizeof...(TObservables) >= 1)
    {
        return std::move(*static_cast<SpecificObservable*>(this)).op(merge_with_impl<Type>(std::forward<TObservables>(observables)...));
    }
};
} // namespace rpp::details
