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

#include <rpp/observables/constraints.hpp>             // constraint::observable_of_type
#include <rpp/observables/details/member_overload.hpp> // member_overload


namespace rpp::details
{
struct concat_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type>
struct concat_impl;

template<constraint::decayed_type Type, constraint::observable_of_type<Type> ... TObservables>
auto concat_with_impl(TObservables&&... observables);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, concat_tag>
{
   /**
    * \brief Converts observable of observables of items into observable of items via merging emissions but without overlapping (current observable completes THEN next started to emit its values)
    * 
    * \marble concat
    {
        source observable : 
        {   
            +--1-2-3-|
            .....+4--6-|
        }
        operator "concat" : +--1-2-3-4--6-|
    }
    *
    * \details Actually it subscribes on first observable from emissions. When first observable completes, then it subscribes on second observable from emissions and etc...
    *
    * \return new specific_observable with the concat operator as most recent operator.
    * \warning #include <rpp/operators/concat.hpp>
    * 
    * \par Example
    * \snippet concat.cpp concat
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to store observables (== emissions) and some internal variables
    *    - Wraps subscriber with serialization logic to be sure callbacks called serialized
    * - <b>OnNext for original observable</b>
    *    - If no any active observable, then subscribes on new obtained observable, else place it in queue
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted from original observable</b>
    *    - Just forwards original on_completed if no any active observable (else we need to processa all observables from queue and they would emit on_completed for subscriber)
    * - <b>OnCompleted from inner observable</b>
    *    - Subscribe on next observable from queue (if any)
    *
    * \ingroup aggregate_operators
    * \see https://reactivex.io/documentation/operators/concat.html
    */
    template<typename ...Args>
    auto concat() const& requires (is_header_included<concat_tag, Args...>&& rpp::constraint::observable<Type>)
    {
        return static_cast<const SpecificObservable*>(this)->template lift<utils::extract_observable_type_t<Type>>(concat_impl<Type>{});
    }

    template<typename ...Args>
    auto concat() && requires (is_header_included<concat_tag, Args...>&& rpp::constraint::observable<Type>)
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<utils::extract_observable_type_t<Type>>(concat_impl<Type>{});
    }

    /**
    * \brief Combines submissions from current observable with other observables into one but without overlapping (current observable completes THEN next started to emit its values)
    *
    * \marble concat_with
        {
            source original_observable: +--1-2-3-|
            source second:              +-4--6-|
            operator "concat_with" :    +--1-2-3--4--6-|
        }
    *
    * \details Actually this operator subscribes on original observable. When original observable completes, then it subscribes on first observable from arguments and etc...
    *
    * \return new specific_observable with the concat operator as most recent operator.
    * \warning #include <rpp/operators/concat.hpp>
    * 
    * \par Example
    * \snippet concat.cpp concat_with
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to store observables (== emissions) and some internal variables
    *    - Wraps subscriber with serialization logic to be sure callbacks called serialized
    * - <b>OnNext</b>
    *    - Just forwards on_next
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted from original observable</b>
    *    - Just forwards original on_completed if no any active observable (else we need to processa all observables from queue and they would emit on_completed for subscriber)
    * - <b>OnCompleted from inner observable</b>
    *    - Subscribe on next observable from queue (if any)
    *
    * \ingroup aggregate_operators
    * \see https://reactivex.io/documentation/operators/concat.html
    */
    template<constraint::observable_of_type<Type> ...TObservables>
    auto concat_with(TObservables&&... observables) const& requires (is_header_included<concat_tag, TObservables...> && sizeof...(TObservables) >= 1)
    {
        return concat_with_impl<Type>(static_cast<const SpecificObservable*>(this)->as_dynamic(), std::forward<TObservables>(observables).as_dynamic()...);
    }

    template<constraint::observable_of_type<Type> ...TObservables>
    auto concat_with(TObservables&&... observables) && requires (is_header_included<concat_tag, TObservables...> && sizeof...(TObservables) >= 1)
    {
        return concat_with_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)).as_dynamic(), std::forward<TObservables>(observables).as_dynamic()...);
    }
};
} // namespace rpp::details
