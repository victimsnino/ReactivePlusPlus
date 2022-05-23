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

#include <rpp/observables/constraints.hpp>          // own constraints
#include <rpp/observables/fwd.hpp>                  // own forwarding
#include <rpp/operators/fwd.hpp>                    // forwarding of member_overaloads
#include <rpp/operators/lift.hpp>                   // must-have operators
#include <rpp/observables/blocking_observable.hpp>  // as_blocking

#include <type_traits>

/**
 * \defgroup observables Observables
 * \brief Observable is the source of any Reactive Stream. Observable provides ability to subscribe observer on some events.
 * \see https://reactivex.io/documentation/observable.html
 */

/**
* \defgroup operators Operators
* \brief Operators is way to modify observables and extend with some extra custom logic
* \see https://reactivex.io/documentation/operators.html 
*/

namespace rpp::details
{
struct observable_tag {};

template<typename T, typename TObservable>
concept op_fn = constraint::observable<std::invoke_result_t<T, TObservable>>;
} // namespace rpp::details

namespace rpp
{
/** 
 * \brief Interface of observable
 * \tparam Type type provided by this observable
 */
template<constraint::decayed_type Type>
struct virtual_observable : public details::observable_tag
{
    virtual ~virtual_observable() = default;
};

/**
 * \brief Base part of observable. Mostly used to provide some interface functions used by all observables
 * \tparam Type type provided by this observable 
 * \tparam SpecificObservable final type of observable inherited from this observable to successfully copy/move it
 */
template<constraint::decayed_type Type, typename SpecificObservable>
struct interface_observable
    : public virtual_observable<Type>
    , details::member_overload<Type, SpecificObservable, details::lift_tag>
    , details::member_overload<Type, SpecificObservable, details::map_tag>
    , details::member_overload<Type, SpecificObservable, details::filter_tag>
    , details::member_overload<Type, SpecificObservable, details::take_tag>
    , details::member_overload<Type, SpecificObservable, details::take_while_tag>
    , details::member_overload<Type, SpecificObservable, details::merge_tag>
    , details::member_overload<Type, SpecificObservable, details::observe_on_tag>
    , details::member_overload<Type, SpecificObservable, details::publish_tag>
    , details::member_overload<Type, SpecificObservable, details::multicast_tag>
    , details::member_overload<Type, SpecificObservable, details::repeat_tag>
    , details::member_overload<Type, SpecificObservable, details::subscribe_on_tag>
    , details::member_overload<Type, SpecificObservable, details::with_latest_from_tag>
{
public:
    /**
    * \brief The apply function to observable which returns observable of another type
    * \tparam OperatorFn type of function which applies to this observable
    * \return new specific_observable of NewType
    * \ingroup operators
    * 
    */
    template<details::op_fn<SpecificObservable> OperatorFn>
    auto op(OperatorFn&& fn) const &
    {
        return fn(CastThis());
    }

    template<details::op_fn<SpecificObservable> OperatorFn>
    auto op(OperatorFn&& fn) &&
    {
        return fn(MoveThis());
    }

    /**
     * \brief Converts existing observable to rpp::blocking_observable which has another interface and abilities
     */
    auto as_blocking() const &
    {
        return blocking_observable{ CastThis ()};
    }

    auto as_blocking()&&
    {
        return blocking_observable{ MoveThis() };
    }
    
private:
    const SpecificObservable& CastThis() const
    {
        return *static_cast<const SpecificObservable*>(this);
    }

    SpecificObservable&& MoveThis()
    {
        return std::move(*static_cast<SpecificObservable*>(this));
    }
};
} // namespace rpp
