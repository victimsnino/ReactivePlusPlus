//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                    TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/observables/details/member_overload.hpp>

#include <vector>

namespace rpp::details
{
struct buffer_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type>
using buffer_bundle_type = std::vector<Type>;

template<constraint::decayed_type Type>
struct buffer_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, buffer_tag>
{
    /**
    * \brief periodically gather items emitted by an Observable into bundles and emit these bundles rather than emitting
    * the items one at a time
    *
    * \marble buffer
        {
            source observable    : +-1-2-3-|
            operator "buffer(2)" : +---{1,2}-{3}-|
        }
    *
    * \details the resulting bundle is std::vector. Actually it is similar to `window` but it emits vectors instead of observables.
    *
    * \param count number of items being bundled.
    * \return new specific_observable with the buffer operator as most recent operator.
    * \warning #include <rpp/operators/buffer.hpp>
    * 
    * \par Example:
    * \snippet buffer.cpp buffer
    * 
    * \ingroup transforming_operators
    * \see https://reactivex.io/documentation/operators/buffer.html
    */
    template<typename ...Args>
    auto buffer(size_t count) const & requires is_header_included<buffer_tag, Args...>
    {
        return CastThis()->template lift<buffer_bundle_type<Type>>(buffer_impl<Type>{count});
    }

    template<typename ...Args>
    auto buffer(size_t count) && requires is_header_included<buffer_tag, Args...>
    {
        return MoveThis().template lift<buffer_bundle_type<Type>>(buffer_impl<Type>{count});
    }

private:
    const SpecificObservable* CastThis() const
    {
        return static_cast<const SpecificObservable*>(this);
    }

    SpecificObservable&& MoveThis()
    {
        return std::move(*static_cast<SpecificObservable*>(this));
    }
};
} // namespace rpp::details
