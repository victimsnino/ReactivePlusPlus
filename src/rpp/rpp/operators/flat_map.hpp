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

#include <rpp/operators/map.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/operators/fwd/flat_map.hpp>

IMPLEMENTATION_FILE(flat_map_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, flat_map_callable<Type> Callable>
auto flat_map_impl(auto&& observable, Callable&& callable)
{
    return std::forward<decltype(observable)>(observable)
           .map(std::forward<Callable>(callable))
           .merge();
}
} // namespace rpp::details
