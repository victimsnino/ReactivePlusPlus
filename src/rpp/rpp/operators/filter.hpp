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

#include <rpp/defs.hpp>                     // RPP_NO_UNIQUE_ADDRESS
#include <rpp/operators/lift.hpp>           // required due to operator uses lift
#include <rpp/operators/fwd/filter.hpp>     // own forwarding
#include <rpp/subscribers/constraints.hpp>  // constraint::subscriber_of_type
#include <rpp/utils/utilities.hpp>          // utils::as_const

#include <utility>

IMPLEMENTATION_FILE(filter_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, std::predicate<const Type&> Predicate>
struct filter_impl_on_next
{
    RPP_NO_UNIQUE_ADDRESS Predicate predicate;

    template<typename TVal, constraint::subscriber_of_type<Type> TSub>
    void operator()(TVal&& value, const TSub& subscriber) const
    {
        if (predicate(utils::as_const(value)))
            subscriber.on_next(std::forward<TVal>(value));
    }
};

template<constraint::decayed_type Type, std::predicate<const Type&> Predicate>
struct filter_impl
{
    RPP_NO_UNIQUE_ADDRESS filter_impl_on_next<Type, Predicate> on_next;

    template<constraint::subscriber TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  on_next,
                                                  utils::forwarding_on_error{},
                                                  utils::forwarding_on_completed{},
                                                  std::forward<TSub>(subscriber));
    }
};
} // namespace rpp::details
