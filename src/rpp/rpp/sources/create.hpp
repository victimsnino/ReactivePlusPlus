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

#include <rpp/observables/specific_observable.hpp>
#include <rpp/sources/fwd.hpp>

#include <type_traits>

IMPLEMENTATION_FILE(create_tag);

namespace rpp::observable
{
/**
 * \brief Creates rpp::specific_observable with passed action as OnSubscribe
 * 
 * \marble create
   {
       operator "create:  on_next(1), on_next(3), on_completed()": +--1--3--|
   }
 * 
 * \tparam Type manually specified type of value provided by this observable
 * \param on_subscribe is action called after subscription on this observable
 * \return rpp::specific_observable with passed action
 *
 * \par Examples:
 * \snippet create.cpp create
 * \snippet create.cpp create with capture
 * \snippet create.cpp create type deduction
 *
 * \ingroup creational_operators
 * \see https://reactivex.io/documentation/operators/create.html
 */
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe) requires rpp::details::is_header_included<rpp::details::create_tag, Type, OnSubscribeFn>
{
    using CreatedOnSubscribeFn = std::decay_t<OnSubscribeFn>;
    #if defined(RPP_TYPE_ERASED_OBSERVABLE) && RPP_TYPE_ERASED_OBSERVABLE
        return specific_observable<Type>{std::forward<OnSubscribeFn>(on_subscribe)};
    #else
        return specific_observable<Type, CreatedOnSubscribeFn>{std::forward<OnSubscribeFn>(on_subscribe)};
    #endif
}

/**
 * \brief Creates rpp::specific_observable with passed action as OnSubscribe
 * 
 * \warning this overloading deduce type of observable from passed function argument
 * 
 * \marble create
   {
       operator "create:  on_next(1), on_next(3), on_completed()": +--1--3--|
   }
 * 
 * \param on_subscribe is action called after subscription on this observable
 * \return rpp::specific_observable with passed action
 *
 * \par Examples:
 * \snippet create.cpp create
 * \snippet create.cpp create with capture
 * \snippet create.cpp create type deduction
 *
 * \ingroup creational_operators
 * \see https://reactivex.io/documentation/operators/create.html
 */
template<utils::is_callable OnSubscribeFn, constraint::decayed_type Type>
    requires constraint::on_subscribe_fn<OnSubscribeFn, Type>
auto create(OnSubscribeFn&& on_subscribe) requires rpp::details::is_header_included<rpp::details::create_tag, Type, OnSubscribeFn>
{
    return create<Type>(std::forward<OnSubscribeFn>(on_subscribe));
}
} // namespace rpp::observable