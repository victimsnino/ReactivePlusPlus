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

#include <rpp/memory_model.hpp>
#include <rpp/observables/constraints.hpp>
#include <rpp/observables/details/member_overload.hpp>

namespace rpp::details
{
struct start_with_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObservable, constraint::observable_of_type<Type> ...TObservables>
auto start_with_impl(TObservable&& observable, TObservables&&... observables_to_start_with);

template<rpp::memory_model memory_model, constraint::decayed_type Type, constraint::decayed_same_as<Type> ...TTypes, constraint::observable_of_type<Type> TObservable>
auto start_with_impl(TObservable&& observable, TTypes&& ...vals_to_start_with);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, start_with_tag>
{
    /**
    * \brief Combines submissions from current observable with values into one but without overlapping and starting from values provided as arguments
    *
    * \marble start_with
        {
            source original_observable   : +-4--6-|
            operator "start_with(1,2,3)" : +-1-2-3--4--6-|
        }
    *
    * \details Actually it makes concat(rpp::source::just(vals_to_start_with)..., current_observable) so observables from argument subscribed before current observable
    *
    * \tparam memory_model memory_model strategy used to store provided values
    * \param vals list of values which should be emitted before current observable
    *
    * \return new specific_observable with the start_with operator as most recent operator.
    * \warning #include <rpp/operators/start_with.hpp>
    *
    * \par Example
    * \snippet start_with.cpp start_with
    *
    * \ingroup combining_operators
    * \see https://reactivex.io/documentation/operators/startwith.html
     */
    template<rpp::memory_model memory_model = memory_model::use_stack, constraint::decayed_same_as<Type> ...TTypes>
    auto start_with(TTypes&&...vals_to_start_with) const & requires is_header_included<start_with_tag, TTypes...>
    {
        return start_with_impl<memory_model, Type>(*static_cast<const SpecificObservable*>(this), std::forward<TTypes>(vals_to_start_with)...);
    }

    template<rpp::memory_model memory_model = memory_model::use_stack, constraint::decayed_same_as<Type> ...TTypes>
    auto start_with(TTypes&&...vals_to_start_with) && requires (is_header_included<start_with_tag, TTypes...>&& constraint::variadic_is_same_type<Type, TTypes...>)
    {
        return  start_with_impl<memory_model, Type>(std::move(*static_cast<SpecificObservable*>(this)), std::forward<TTypes>(vals_to_start_with)...);
    }

    /**
    * \brief Combines submissions from current observable with other observables into one but without overlapping and starting from observables provided as arguments
    *
    * \marble start_with_observable
        {
            source original_observable       : +-4--6-|
            operator "start_with(-1-2-3-|)"  : +--1-2-3--4--6-|
        }
    *
    * \details Actually it makes concat(observables_to_start_with..., current_observable) so observables from argument subscribed before current observable
    *
    * \param observables list of observables which should be used before current observable
    *
    * \return new specific_observable with the start_with operator as most recent operator.
    * \warning #include <rpp/operators/start_with.hpp>
    *
    * \par Example
    * \snippet start_with.cpp start_with observable
    *
    * \ingroup combining_operators
    * \see https://reactivex.io/documentation/operators/startwith.html
    */
    template<constraint::observable_of_type<Type> ...TObservables>
    auto start_with(TObservables&&...observables_to_start_with) const& requires is_header_included<start_with_tag, TObservables...>
    {
        return start_with_impl<Type>(*static_cast<const SpecificObservable*>(this), std::forward<TObservables>(observables_to_start_with)...);
    }

    template<constraint::observable_of_type<Type> ...TObservables>
    auto start_with(TObservables&&...observables_to_start_with) && requires is_header_included<start_with_tag, TObservables...>
    {
        return start_with_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)), std::forward<TObservables>(observables_to_start_with)...);
    }
};
} // namespace rpp::details
