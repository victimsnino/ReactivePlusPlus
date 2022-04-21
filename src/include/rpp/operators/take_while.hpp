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

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/take_while.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/utilities.hpp>

IMPLEMENTATION_FILE(take_while_tag);

namespace rpp::operators
{
template<typename Predicate>
auto take_while(Predicate&& predicate) requires details::is_header_included<details::take_while_tag, Predicate>
{
    return [predicate = std::forward<Predicate>(predicate)]<constraint::observable TObservable>(TObservable && observable)
    {
        return observable.take_while(predicate);
    };
}
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
template<std::predicate<const Type&> Predicate>
auto member_overload<Type, SpecificObservable, take_while_tag>::take_while_impl(Predicate&& predicate)
{
    return [predicate = std::forward<Predicate>(predicate)](auto&& value, const constraint::subscriber_of_type<Type> auto& subscriber)
    {
        if (predicate(utils::as_const(value)))
            subscriber.on_next(std::forward<decltype(value)>(value));
        else
            subscriber.on_completed();
    };
}
} // namespace rpp::details
