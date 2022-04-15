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

#include <concepts>
#include <exception>


namespace rpp::constraint
{
template<typename T, typename Type> concept decayed_same_as      = std::same_as<std::decay_t<T>, std::decay_t<Type>>;

template<typename T> concept decayed_type = std::same_as<std::decay_t<T>, T>;

template<typename Type, typename...Types> concept variadic_is_same_type = sizeof...(Types) == 1 && (decayed_same_as<Type, Types> && ...);
} // namespace rpp::constraint