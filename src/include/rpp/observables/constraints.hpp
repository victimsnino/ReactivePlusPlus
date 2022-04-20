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

#include <rpp/observables/fwd.hpp>
#include <rpp/observables/type_traits.hpp>

#include <type_traits>

namespace rpp::constraint
{
template<typename T> concept observable = std::is_base_of_v<details::observable_tag, std::decay_t<T>>;

template<typename T, typename Type> concept observable_of_type = observable<T> && std::is_same_v<utils::extract_observable_type_t<T>, Type>;
} // namespace rpp::constraint