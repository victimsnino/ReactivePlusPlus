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

#include <rpp/subscribers/fwd.h>
#include <rpp/subscribers/type_traits.h>

#include <type_traits>

namespace rpp::constraint
{
template<typename T> concept subscriber = std::is_base_of_v<details::subscriber_tag, std::decay_t<T>>;

template<typename T, typename Type> concept subscriber_of_type = subscriber<T> && std::is_same_v<utils::extract_subscriber_type_t<T>, Type>;

} // namespace rpp::constraint