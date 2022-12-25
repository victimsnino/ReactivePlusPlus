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
#include <rpp/utils/functors.hpp>
#include <rpp/utils/function_traits.hpp>


#include <tuple>

namespace rpp::details
{
struct with_latest_from_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, typename TSelector, constraint::observable ...TObservables>
struct with_latest_from_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, with_latest_from_tag>
{
    /**
    * \brief Combines latest emissions from observables with emission from current observable when it sends new value via applying selector
    * 
    * \marble with_latest_from_custom_selector
      {
          source observable                                 : +------1    -2    -3    -|
          source other_observable                           : +-5-6-7-    --    8-    -|
          operator "with_latest_from: x,y =>std::pair{x,y}" : +------{1,5}-{2,7}-{3,8}-|
      }
    *
    * \details Actually this operator just keeps last values from all other observables and combines them together with each new emission from original observable
    * 
    * \param selector is applied to current emission of current observable and latests emissions from observables
    * \param observables are observables whose emissions would be combined when current observable sends new value
    * \return new specific_observable with the with_latest_from operator as most recent operator.
    * \warning #include <rpp/operators/with_latest_from.hpp>
    *
    * \par Examples
    * \snippet with_latest_from.cpp with_latest_from custom selector
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to keep last values from all observables
    * - <b>OnNext for original observable</b>
    *    - Applies selector to new emission and all saved last values from other observable (if any value for all observables)
    * - <b>OnNext other original observables</b>
    *    - Just updates last value for this observable
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted for original observable</b>
    *    - Just forwards original on_completed 
    * - <b>OnCompleted for other observables</b>
    *    - None
    *
    * \ingroup combining_operators
    * \see https://reactivex.io/documentation/operators/combinelatest.html
    */
    template<constraint::observable ...TObservables, std::invocable<Type, utils::extract_observable_type_t<TObservables>...> TSelector>
    auto with_latest_from(TSelector&& selector, TObservables&&...observables) const& requires is_header_included<with_latest_from_tag, TObservables...>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<utils::decayed_invoke_result_t<TSelector, Type, utils::extract_observable_type_t<TObservables>...>>(with_latest_from_impl<Type, std::decay_t<TSelector>, std::decay_t<TObservables>...>{std::forward<TSelector>(selector), { std::forward<TObservables>(observables)... }});
    }

    template<constraint::observable ...TObservables, std::invocable<Type, utils::extract_observable_type_t<TObservables>...> TSelector>
    auto with_latest_from(TSelector&& selector, TObservables&&...observables) && requires is_header_included<with_latest_from_tag, TObservables...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<utils::decayed_invoke_result_t<TSelector, Type, utils::extract_observable_type_t<TObservables>...>>(with_latest_from_impl<Type, std::decay_t<TSelector>, std::decay_t<TObservables>...>{std::forward<TSelector>(selector), { std::forward<TObservables>(observables)... }});
    }

    /**
    * \brief Combines latest emissions from observables with emission from current observable when it sends new value via making tuple
    * 
    * \marble with_latest_from
      {
          source observable                       : +------1    -2    -3    -|
          source other_observable                 : +-5-6-7-    --    8-    -|
          operator "with_latest_from: make_tuple" : +------{1,5}-{2,7}-{3,8}-|
      }
    * 
    * \param observables are observables whose emissions would be combined when current observable sends new value
    * \return new specific_observable with the with_latest_from operator as most recent operator.
    * \warning #include <rpp/operators/with_latest_from.hpp>
    *
    * \par Examples
    * \snippet with_latest_from.cpp with_latest_from
    *
    * \ingroup combining_operators
    * \see https://reactivex.io/documentation/operators/combinelatest.html
    */
    template<constraint::observable ...TObservables>
    auto with_latest_from(TObservables&&...observables) const& requires is_header_included<with_latest_from_tag, TObservables...>
    {
        return static_cast<const SpecificObservable*>(this)->with_latest_from(utils::pack_to_tuple{}, std::forward<TObservables>(observables)...);
    }

    template<constraint::observable ...TObservables>
    auto with_latest_from(TObservables&&...observables) && requires is_header_included<with_latest_from_tag, TObservables...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).with_latest_from(utils::pack_to_tuple{}, std::forward<TObservables>(observables)...);
    }
};
} // namespace rpp::details
