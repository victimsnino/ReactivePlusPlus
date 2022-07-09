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

namespace rpp::details
{
struct flat_map_tag;
}

namespace rpp::details
{
template<typename Fn, typename Type>
concept flat_map_callable = std::invocable<Fn, Type> && constraint::observable<std::invoke_result_t<Fn, Type>>;

template<constraint::decayed_type Type, flat_map_callable<Type> Callable>
auto flat_map_impl(auto&& observable, Callable&& callable);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, flat_map_tag>
{
    /**
    * \brief Transform emissions to observables via provided function and then merge emissions from such an observables
    *
    * \warning According to observable contract (https://reactivex.io/documentation/contract.html) emissions from any observable should be serialized, so, resulting observable uses mutex to satisfy this requirement
    * 
    * \marble flat_map
        {
            source observable                   : +--1--2--3--|
            operator "flat_map: x=>just(x,x+1)" : +--12-23-34-|
        }
    *
    * \details Actually it makes `map` and then `merge`.
    *
    * \param callable Function to transform item to observable
    * \return new specific_observable with the flat_map operator as most recent operator.
    * \warning #include <rpp/operators/flat_map.hpp>
    * 
    * \par Example:
    * \snippet flat_map.cpp flat_map
    * 
    * \ingroup transforming_operators
    * \see https://reactivex.io/documentation/operators/flatmap.html
    */
    template<flat_map_callable<Type> Callable>
    auto flat_map(Callable&& callable) const & requires is_header_included<flat_map_tag, Callable>
    {
        return flat_map_impl<Type>(*static_cast<const SpecificObservable*>(this), std::forward<Callable>(callable));
    }

    template<flat_map_callable<Type> Callable>
    auto flat_map(Callable&& callable) && requires is_header_included<flat_map_tag, Callable>
    {
        return flat_map_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)), std::forward<Callable>(callable));
    }
};
} // namespace rpp::details
