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
struct publish_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto publish_impl(TObs&& observable);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, publish_tag>
{
    /**
    * \brief Converts ordinary observable to rpp::connectable_observable with help of rpp::subjects::publish_subject
    * \details Connectable observable is common observable, but actually it starts emissions of items only after call "connect", "ref_count" or any other available way. Also it uses subject to multicast values to subscribers
    *
    * \return new specific_observable with the publish operator as most recent operator.
    * \warning #include <rpp/operators/publish.hpp>
    * 
    * \par Example
    * \snippet publish.cpp publish
    *
    * \ingroup connectable_operators
    * \see https://reactivex.io/documentation/operators/publish.html
    */
    template<typename ...Args>
    auto publish() const& requires is_header_included<publish_tag, Args...>
    {
        return publish_impl<Type>(*static_cast<const SpecificObservable*>(this));
    }

    template<typename ...Args>
    auto publish() && requires is_header_included<publish_tag, Args...>
    {
        return publish_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)));
    }
};
} // namespace rpp::details
