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
#include <rpp/subjects/constraints.hpp>

namespace rpp::details
{
struct multicast_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs, subjects::constraint::subject_of_type<Type> TSubject>
auto multicast_impl(TObs&& observable, TSubject&& subject);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, multicast_tag>
{
    /**
    * \brief converts ordinary observable to Connectable Observable with help of provided subject
    * \details Connectable observable is common observable, but actually it starts emissions of items only after call "connect", "ref_count" or any other available way. Also it uses subject to multicast values to subscribers
    *
    * \snippet multicast.cpp multicast
    *
    * \see https://reactivex.io/documentation/operators/publish.html
    *
    * \return new specific_observable with the multicast operator as most recent operator.
    * \warning #include <rpp/operators/multicast.hpp>
    * \ingroup operators
    */
    template<subjects::constraint::subject_of_type<Type> TSubject>
    auto multicast(TSubject&& subject) const& requires is_header_included<multicast_tag, TSubject>
    {
        return multicast_impl<Type>(*static_cast<const SpecificObservable*>(this), std::forward<TSubject>(subject));
    }

    template<subjects::constraint::subject_of_type<Type> TSubject>
    auto multicast(TSubject&& subject) && requires is_header_included<multicast_tag, TSubject>
    {
        return multicast_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)), std::forward<TSubject>(subject));
    }
};
} // namespace rpp::details
