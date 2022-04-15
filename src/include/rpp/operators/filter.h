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

#include <rpp/observables/constraints.h>
#include <rpp/operators/fwd/filter.h>
#include <rpp/subscribers/constraints.h>
#include <rpp/utils/utilities.h>


#include <utility>

IMPLEMENTATION_FILE(filter_tag);

namespace rpp::operators
{
template<typename Predicate>
auto filter(Predicate&& predicate) requires details::is_header_included<details::filter_tag, Predicate>
{
    return [predicate = std::forward<Predicate>(predicate)]<constraint::observable TObservable>(TObservable && observable)
    {
        return observable.filter(predicate);
    };
}
} // namespace rpp::operators
namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
template<std::predicate<const Type&> Predicate>
auto member_overload<Type, SpecificObservable, filter_tag>::filter_impl(Predicate&& predicate)
{
    return [predicate = std::forward<Predicate>(predicate)](auto&& value, const constraint::subscriber auto& subscriber)
    {
        if (predicate(utilities::as_const(value)))
            subscriber.on_next(std::forward<decltype(value)>(value));
    };
}
} // namespace rpp::details
