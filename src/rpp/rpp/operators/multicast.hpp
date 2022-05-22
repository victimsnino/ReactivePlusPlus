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
#include <rpp/operators/fwd/multicast.hpp>
#include <rpp/observables/connectable_observable.hpp>

IMPLEMENTATION_FILE(multicast_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs, subjects::constraint::subject_of_type<Type> TSubject>
auto multicast_impl(TObs&& observable, TSubject&& subject)
{
    return connectable_observable<Type, std::decay_t<TSubject>, std::decay_t<TObs>>{std::forward<TObs>(observable), std::forward<TSubject>(subject)};
}
} // namespace rpp::details
