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
struct take_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, take_tag>
{
   /**
    * \brief Emit only first `count` items provided by observable, then send `on_completed`
    *
    * \marble take
    {
        source observable  : +--1-2-3-4-5-6-|
        operator "take(3)" : +--1-2-3|
    }
    * \details Actually this operator just emits emissions while counter is not zero and decrements counter on each emission
    *
    * \param count amount of items to be emitted. 0 - instant complete
    * \return new specific_observable with the Take operator as most recent operator.
    * \warning #include <rpp/operators/take.hpp>
    * 
    * \par Example:
    * \snippet take.cpp take
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocate one `shared_ptr` to store counter
    * - <b>OnNext</b>
    *    - Just forwards emission if counter is not zero
    *    - Decrements counter if not zero
    *    - If counter reached zero, then emits OnCompleted
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - Just forwards original on_completed 
    *
    * \ingroup filtering_operators
    * \see https://reactivex.io/documentation/operators/take.html
    */
    template<typename...Args>
    auto take(size_t count) const & requires is_header_included<take_tag, Args...>
    {
        return cast_this()->template lift<Type>(take_impl<Type>{count});
    }

    template<typename...Args>
    auto take(size_t count) && requires is_header_included<take_tag, Args...>
    {
        return move_this().template lift<Type>(take_impl<Type>{count});
    }

private:
    const SpecificObservable* cast_this() const
    {
        return static_cast<const SpecificObservable*>(this);
    }

    SpecificObservable&& move_this()
    {
        return std::move(*static_cast<SpecificObservable*>(this));
    }
};
} // namespace rpp::details
