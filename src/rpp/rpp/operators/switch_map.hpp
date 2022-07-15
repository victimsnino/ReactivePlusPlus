//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                    TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/operators/fwd/switch_map.hpp>

#include <rpp/operators/map.hpp>
#include <rpp/operators/switch_on_next.hpp>

IMPLEMENTATION_FILE(switch_map_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, switch_map_callable<Type> Callable>
auto switch_map_impl(auto &&observable, Callable &&callable)
{
    return std::forward<decltype(observable)>(observable)
            .map(std::forward<Callable>(callable))
            .switch_on_next();
}
} // namespace rpp::details
