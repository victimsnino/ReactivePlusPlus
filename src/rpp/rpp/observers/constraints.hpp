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

#include <rpp/observers/fwd.hpp>
#include <rpp/observers/type_traits.hpp>

#include <type_traits>

namespace rpp::details
{
struct subscriber_tag;
} // namespace rpp::details

namespace rpp::constraint
{
template<typename T> concept observer = std::is_base_of_v<details::observer_tag, std::decay_t<T>> && !std::is_base_of_v<details::subscriber_tag, std::decay_t<T>>;

template<typename T> concept decayed_observer                = observer<T> && decayed_type<T>;
template<typename T, typename Type> concept observer_of_type = observer<T> && std::is_same_v<utils::extract_observer_type_t<T>, Type>;
} // namespace rpp::constraint
