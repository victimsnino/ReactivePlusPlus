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

#include <rpp/utils/constraints.hpp>

namespace rpp::details
{
struct map_tag;

template<constraint::decayed_type Type, std::invocable<Type> Callable>
struct map_impl;
} // namespace rpp::details
