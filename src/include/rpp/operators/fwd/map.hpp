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
#include <rpp/observables/constraints.hpp>

namespace rpp::details
{
struct map_tag;
}

namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::map
 */
template<typename Callable>
auto map(Callable&& callable) requires details::is_header_included<details::map_tag, Callable>
{
    return[callable = std::forward<Callable>(callable)]<constraint::observable TObservable>(TObservable && observable)
    {
        return std::forward<TObservable>(observable).map(callable);
    };
}
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, std::invocable<Type> Callable>
auto map_impl(Callable&& callable);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, map_tag>
{
    /**
     * \brief transform the items emitted by an Observable by applying a function to each item
     *
     * \details The Map operator applies a function of your choosing to each item emitted by the source Observable, and returns an Observable that emits the results of these function applications.
     *
     * The Map operator can keep same type of value or change it to some another type.
     *
     * Example with same type:
     * \snippet map.cpp Same type
     *
     * Example with changed type:
     * \snippet map.cpp Changed type
     *
     * \see https://reactivex.io/documentation/operators/map.html
     *
     * \tparam Callable type of callable used to provide this transformation
     * \return new specific_observable with the Map operator as most recent operator.
     * \warning #include <rpp/operators/map.hpp>
     * \ingroup operators
     */
    template<std::invocable<Type> Callable>
    auto map(Callable&& callable) const & requires is_header_included<map_tag, Callable>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<std::invoke_result_t<Callable, Type>>(map_impl<Type>(std::forward<Callable>(callable)));
    }

    template<std::invocable<Type> Callable>
    auto map(Callable&& callable) && requires is_header_included<map_tag, Callable>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<std::invoke_result_t<Callable, Type>>(map_impl<Type>(std::forward<Callable>(callable)));
    }
};
} // namespace rpp::details
