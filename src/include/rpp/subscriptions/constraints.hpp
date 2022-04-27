//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include "rpp/utils/constraints.hpp"

#include <rpp/subscriptions/fwd.hpp>

#include <concepts>

namespace rpp::constraint
{
template<typename T>
concept subscription = std::derived_from<T, subscription_base> || decayed_same_as<T, subscription_base>;
} // namespace rpp::constraint