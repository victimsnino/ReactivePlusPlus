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
#include <type_traits>

namespace rpp::utilities
{
template<class T>
constexpr std::add_const_t<T>& as_const(const T& v) noexcept { return v; }

template<class T>
constexpr T&& as_const(T&& v) noexcept requires std::is_rvalue_reference_v<T&&> { return std::forward<T>(v); }

} // namespace rpp::utilities
