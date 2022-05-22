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
#include <rpp/operators/fwd/publish.hpp>
#include <rpp/operators/multicast.hpp>

IMPLEMENTATION_FILE(publish_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto publish_impl(TObs&& observable)
{
    return std::forward<TObs>(observable).multicast(rpp::subjects::publish_subject<Type>{});
}
} // namespace rpp::details
