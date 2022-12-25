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
    struct skip_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type>
struct skip_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, skip_tag>
{
   /**
    * \brief Skip first `count` items provided by observable then send rest items as expected
    *
    * \marble skip
    {
        source observable  : +--1-2-3-4-5-6-|
        operator "skip(3)" : +--------4-5-6-|
    }
    *
    * \details Actually this operator just decrements counter and starts to forward emissions when counter reaches zero.
    *
    * \param count amount of items to be skipped
    * \return new specific_observable with the skip operator as most recent operator.
    * \warning #include <rpp/operators/skip.hpp>
    *
    * \par Example:
    * \snippet skip.cpp skip
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to store counter
    * - <b>OnNext</b>
    *    - Forwards emission if counter is zero
    *    - Decrements counter if not zero
    * - <b>OnError</b>
    *    - Just forwards original on_error
    * - <b>OnCompleted</b>
    *    - Just forwards original on_completed
    *
    * \ingroup filtering_operators
    * \see https://reactivex.io/documentation/operators/skip.html
    */
    template<typename...Args>
    auto skip(size_t count) const& requires is_header_included<skip_tag, Args...>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(skip_impl<Type>{count});
    }

    template<typename...Args>
    auto skip(size_t count) && requires is_header_included<skip_tag, Args...>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(skip_impl<Type>{count});
    }
};
} // namespace rpp::details
