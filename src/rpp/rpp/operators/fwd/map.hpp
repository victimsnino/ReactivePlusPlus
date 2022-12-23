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
#include <rpp/utils/function_traits.hpp>

namespace rpp::details
{
struct map_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, std::invocable<Type> Callable>
struct map_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, map_tag>
{
   /**
    * \brief Transform the items emitted by an Observable via applying a function to each item and emitting result
    * \note The Map operator can keep same type of value or change it to some another type.
    * 
    * \marble map
    {
        source observable       : +--1   -2   --3   -|
        operator "map: x=>x+10" : +--(11)-(12)--(13)-|
    }
    *
    * \details Actually this operator just applies callable to each obtained emission and emit resulting value
    *
    * \param callable is callable used to provide this transformation. Should accept Type of original observable and return type for new observable
    * \return new specific_observable with the Map operator as most recent operator.
    * \warning #include <rpp/operators/map.hpp>
    * 
    * \par Example with same type:
    * \snippet map.cpp Same type
    *
    * \par Example with changed type:
    * \snippet map.cpp Changed type
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - None
    * - <b>OnNext</b>
    *    - Just forwards result of applying callable to emissions
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - Just forwards original on_completed 
    *
    * \ingroup transforming_operators
    * \see https://reactivex.io/documentation/operators/map.html
    */
    template<std::invocable<Type> Callable>
    auto map(Callable&& callable) const & requires is_header_included<map_tag, Callable>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<utils::decayed_invoke_result_t<Callable, Type>>(map_impl<Type, std::decay_t<Callable>>{std::forward<Callable>(callable)});
    }

    template<std::invocable<Type> Callable>
    auto map(Callable&& callable) && requires is_header_included<map_tag, Callable>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<utils::decayed_invoke_result_t<Callable, Type>>(map_impl<Type, std::decay_t<Callable>>{std::forward<Callable>(callable)});
    }
};
} // namespace rpp::details
