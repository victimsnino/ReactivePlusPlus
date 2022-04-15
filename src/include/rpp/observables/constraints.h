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

#include <rpp/observables/fwd.h>

#include <type_traits>

namespace rpp::constraint
{
template<typename T> concept observable = std::is_base_of_v<details::observable_tag, std::decay_t<T>>;
} // namespace rpp::constraint