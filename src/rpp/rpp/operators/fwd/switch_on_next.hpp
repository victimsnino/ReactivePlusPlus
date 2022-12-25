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
    struct switch_on_next_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type>
struct switch_on_next_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, switch_on_next_tag>
{
    /**
    * \brief Converts observable of observables into observable of values which emits values from most recent underlying observable till new observable obtained
    *
    * \marble switch_on_next
        {
        source observable                : 
        {   
            +--1-2-3-5--|
            .....+4--6-9|
            .......+7-8-|
        }
        operator "switch_on_next" : +--1-24-7-8|
    }
    *
    * \details Actually this operator just unsubscribes from previous observable and subscribes on new observable when obtained in `on_next`
    *
    * \return new specific_observable with the switch_on_next operator as most recent operator.
    * \warning #include <rpp/operators/switch_on_next.hpp>
    *
    * \par Example:
    * \snippet switch_on_next.cpp switch_on_next
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to store internal state
    * - <b>OnNext</b>
    *    - Unsubscribed from previous observable (if any)
    *    - Subscribed on new emitted observable
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - Just forwards original on_completed if no any active inner observable or original observable yet
    *
    * \ingroup combining_operators
    * \see https://reactivex.io/documentation/operators/switch.html
    */
    template<typename ...Args>
    auto switch_on_next() const& requires (is_header_included<switch_on_next_tag, Args...>&& rpp::constraint::observable<Type>)
    {
        return static_cast<const SpecificObservable*>(this)->template lift<utils::extract_observable_type_t<Type>>(switch_on_next_impl<Type>());
    }

    template<typename ...Args>
    auto switch_on_next() && requires (is_header_included<switch_on_next_tag, Args...>&& rpp::constraint::observable<Type>)
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<utils::extract_observable_type_t<Type>>(switch_on_next_impl<Type>());
    }
};
} // namespace rpp::details
