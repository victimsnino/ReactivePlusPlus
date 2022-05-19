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
struct take_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type>
auto take_impl(size_t count);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, take_tag>
{
    /**
     * \brief emit only first Count items provided by observable
     *
     * Example:
     * \snippet take.cpp take
     *
     * \see https://reactivex.io/documentation/operators/take.html
     *
     * \return new specific_observable with the Take operator as most recent operator.
     * \warning #include <rpp/operators/take.hpp>
     * \ingroup operators
     */
    template<typename...Args>
    auto take(size_t count) const & requires is_header_included<take_tag, Args...>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(take_impl<Type>(count));
    }

    template<typename...Args>
    auto take(size_t count) && requires is_header_included<take_tag, Args...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(take_impl<Type>(count));
    }
};
} // namespace rpp::details
