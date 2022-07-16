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

#include <rpp/observables/constraints.hpp>
#include <rpp/observables/details/member_overload.hpp>

namespace rpp::details
{
struct switch_map_tag;
}

namespace rpp::details
{
template<typename Fn, typename Type>
concept switch_map_callable = std::invocable<Fn, Type> && constraint::observable<std::invoke_result_t<Fn, Type>>;

template<constraint::decayed_type Type, switch_map_callable<Type> Callable>
auto switch_map_impl(auto&& observable, Callable&& callable);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, switch_map_tag>
{
    /**
    * \brief convert an Observable that emits Observables into a single Observable that emits the items emitted by the most-recently-emitted of those Observables
    *
    * \marble switch_map
        {
            source observable                 : +--2-3-|
            operator "switch_map: x=>x-x-x-|" : +--22333|
        }
    *
    * \details Actually it makes `map` and then `switch_on_next`.
    *
    * \param callable Function to transform item to observable
    * \return new specific_observable with the switch_map operator as most recent operator.
    * \warning #include <rpp/operators/switch_map.hpp>
    * 
    * \par Example:
    * \snippet switch_map.cpp switch_map
    * 
    * \ingroup transforming_operators
    * \see https://reactivex.io/documentation/operators/switchmap.html
    */
    template<switch_map_callable<Type> Callable>
    auto switch_map(Callable&& callable) const & requires is_header_included<switch_map_tag, Callable>
    {
        return switch_map_impl<Type>(*static_cast<const SpecificObservable*>(this), std::forward<Callable>(callable));
    }

    template<switch_map_callable<Type> Callable>
    auto switch_map(Callable&& callable) && requires is_header_included<switch_map_tag, Callable>
    {
        return switch_map_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)), std::forward<Callable>(callable));
    }
};
} // namespace rpp::details
