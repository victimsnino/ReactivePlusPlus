//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/details/member_overload.hpp>
#include <rpp/observables/constraints.hpp>

#include <tuple>

namespace rpp::details
{
struct with_latest_from_tag;
}

namespace rpp::details
{
struct pack_to_tuple
{
    auto operator()(auto&& ...vals) const { return std::make_tuple(std::forward<decltype(vals)>(vals)...); }
};
template<constraint::decayed_type Type, constraint::observable ...TObservables, std::invocable<Type, utils::extract_observable_type_t<TObservables>...> TSelector >
static auto with_latest_from_impl(TSelector&& selector, TObservables&&...observables);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, with_latest_from_tag>
{
    /**
    * \brief
    *
    * \details
    *	
    * Example:
    *
    * \see 
    *
    * \return new specific_observable with the with_latest_from operator as most recent operator.
    * \warning #include <rpp/operators/with_latest_from.h>
    * \ingroup operators
    */
    template<constraint::observable ...TObservables, std::invocable<Type, utils::extract_observable_type_t<TObservables>...> TSelector>
    auto with_latest_from(TSelector&& selector, TObservables&&...observables) const& requires is_header_included<with_latest_from_tag, TObservables...>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<std::invoke_result_t<TSelector, Type, utils::extract_observable_type_t<TObservables>...>>(with_latest_from_impl<Type>(std::forward<TSelector>(selector), std::forward<TObservables>(observables)...));
    }

    template<constraint::observable ...TObservables, std::invocable<Type, utils::extract_observable_type_t<TObservables>...> TSelector>
    auto with_latest_from(TSelector&& selector, TObservables&&...observables) && requires is_header_included<with_latest_from_tag, TObservables...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<std::invoke_result_t<TSelector, Type, utils::extract_observable_type_t<TObservables>...>>(with_latest_from_impl<Type>(std::forward<TSelector>(selector), std::forward<TObservables>(observables)...));
    }

    template<constraint::observable ...TObservables>
    auto with_latest_from(TObservables&&...observables) const& requires is_header_included<with_latest_from_tag, TObservables...>
    {
        return static_cast<const SpecificObservable*>(this)->with_latest_from(pack_to_tuple{}, std::forward<TObservables>(observables)...);
    }

    template<constraint::observable ...TObservables>
    auto with_latest_from(TObservables&&...observables) && requires is_header_included<with_latest_from_tag, TObservables...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).with_latest_from(pack_to_tuple{}, std::forward<TObservables>(observables)...);
    }
};
} // namespace rpp::details
