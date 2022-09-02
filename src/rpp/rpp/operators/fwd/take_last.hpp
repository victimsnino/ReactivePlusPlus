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

namespace rpp::details
{
struct take_last_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type>
struct take_last_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, take_last_tag>
{
    /**
     * \brief 
     * 
     * \marble take_last
        {
            
        }
     *
     * \return new specific_observable with the take_last operator as most recent operator.
     * \warning #include <rpp/operators/take_last.hpp>
     * 
     * \par Example
     * \snippet take_last.cpp take_last
	 *
     * \ingroup 
     * \see
     */
    template<typename ...Args>
    auto take_last(size_t count) const & requires is_header_included<take_last_tag, Args...>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(take_last_impl<Type>{count});
    }

    template<typename ...Args>
    auto take_last(size_t count) && requires is_header_included<take_last_tag, Args...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(take_last_impl<Type>{count});
    }
};
} // namespace rpp::details
