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
    * \brief Periodically gather emissions emitted by an original Observable into bundles and emit these bundles rather than emitting
    * the items one at a time
    *
    * \marble buffer
        {
            source observable    : +-1-2-3-|
            operator "buffer(2)" : +---{1,2}-{3}-|
        }
    *
    * \details The resulting bundle is `std::vector<Type>` of requested size. Actually it is similar to `window()` operator, but it emits vectors instead of observables.
    *
    * \param count number of items being bundled.
    * \return new specific_observable with the buffer operator as most recent operator.
    * \warning #include <rpp/operators/buffer.hpp>
    * 
    * \par Example:
    * \snippet buffer.cpp buffer
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to store `std::vector<Type>` of requested size.
    * - <b>OnNext</b>
    *    - Accumulates emissions inside current bundle and emits this bundle when requested cound reached and starts new bundle.
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - Emits current active bundle (if any) and just forwards on_completed
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
