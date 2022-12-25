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
    * \brief Emit only last `count` items provided by observable, then send `on_completed`
    * 
    * \marble take_last
    {
        source observable       : +--1-2-3-4-5-6-|
        operator "take_last(3)" : +--------------456|
    }
    *
    * \details Actually this operator has buffer of requested size inside, keeps last `count` values and emit stored values on `on_completed`
    *
    * \param count amount of last items to be emitted
    * \return new specific_observable with the take_last operator as most recent operator.
    * \warning #include <rpp/operators/take_last.hpp>
    * 
    * \par Example
    * \snippet take_last.cpp take_last
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to store internal buffer
    * - <b>OnNext</b>
    *    - Place obtained value into queue
    *    - If queue contains more values than expected - remove oldest one
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - Emits values stored in queue
    *
    * \ingroup filtering_operators
    * \see https://reactivex.io/documentation/operators/takelast.html
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
