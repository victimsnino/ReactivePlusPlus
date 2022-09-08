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

#include <rpp/subscribers/fwd.hpp>   // on_subscribe_fn
#include <rpp/utils/constraints.hpp> // decayed type

namespace rpp::details
{
struct observable_tag{};

template<constraint::decayed_type Type>
struct typed_observable_tag : public details::observable_tag {};
struct dynamic_observable_tag;
} // namespace rpp::details

namespace rpp::constraint
{
template<typename Fn, typename T> concept on_subscribe_fn = std::invocable<std::decay_t<Fn>, dynamic_subscriber<T>>;
} // namespace rpp::constraint

namespace rpp
{

/**
 * \brief Type-full observable (or typed) that has the notion of Type and upstream observables for C++ compiler. e.g. observable<int, map<bool, ...recursive...>> is different from observable<int, filter<int, ...>>.
 *
 * \details This is a C++ technique about de-virtualization. To achieve polymorphic behavior, we could either go for function virtualization or function overload. 
 * However, virtualization is more expensive than function overload in both compile time and runtime. 
 * Therefore, we go for function overload. Actually, we use more advanced functor paradigm for better performance.
 * As a result it has better performance comparing to rpp::dynamic_observable. Use it if possible. But it has worse usability due to OnSubscribeFn template parameter.
 *
 * \param Type is the value type. Observable of type means this source could emit a sequence of items of that "Type".
 * \param OnSubscribeFn is the on_subscribe functor that is called when a subscriber subscribes to this observable. specific_observable stores OnSubscribeFn as member variable, so, it is stored on stack (instead of allocating it on heap).
 * \ingroup observables
 */
#if defined(RPP_TYPE_ERASED_OBSERVABLE) && RPP_TYPE_ERASED_OBSERVABLE
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn = std::function<void(rpp::dynamic_subscriber<Type>)>>
#else
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
#endif
class specific_observable;

/**
 * \brief Type-less observable (or partially untyped) that has the notion of Type but hides the notion of on_subscribe<Type> for C++ compiler.
 *
 * \details This is a C++ technique called type-erasure. Multiple instances of the observable<type> that may have different upstream graphs are considered homogeneous. i.e. They can be stored in the same container, e.g. std::vector.
 * As a result, it uses heap to store on_subscribe and hide its type.
 *
 * \param Type is the value type. Observable of type means this source could emit a sequence of items of that "Type".
 * \ingroup observables
 */
template<constraint::decayed_type Type>
class dynamic_observable;

template<constraint::decayed_type KeyType,
         constraint::decayed_type Type,
         constraint::on_subscribe_fn<Type> OnSubscribeFn>
class grouped_observable;
} // namespace rpp
