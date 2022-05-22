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
struct ref_count_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto ref_count_impl(TObs&& observable);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, ref_count_tag>
{
    /**
    * \brief Forces connectable observable to behave like common observable
    * \details Connects Connectable Observable on the first subscription and unsubscribes on last unsubscription
    *	
    * \snippet ref_count.cpp ref_count
    *
    * \see https://reactivex.io/documentation/operators/refcount.html
    *
    * \return new specific_observable with the ref_count operator as most recent operator.
    * \warning #include <rpp/operators/ref_count.h>
    * \ingroup operators
    */
    template<typename ...Args>
    auto ref_count() const& requires is_header_included<ref_count_tag, Args...>
    {
        return ref_count_impl<Type>(*static_cast<const SpecificObservable*>(this));
    }

    template<typename ...Args>
    auto ref_count() && requires is_header_included<ref_count_tag, Args...>
    {
        return ref_count_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)));
    }
};
} // namespace rpp::details
