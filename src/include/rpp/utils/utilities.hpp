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

#include <atomic>
#include <memory>
#include <type_traits>

namespace rpp::utils
{
template<class T>
constexpr std::add_const_t<T>& as_const(const T& v) noexcept { return v; }

template<class T>
constexpr T&& as_const(T&& v) noexcept requires std::is_rvalue_reference_v<T&&> { return std::forward<T>(v); }

#if __cpp_lib_atomic_shared_ptr
template<typename T>
using atomic_shared_ptr = std::atomic<std::shared_ptr<T>>;
#else
template<typename T>
using atomic_shared_ptr = std::shared_ptr<T>;
#endif
} // namespace rpp::utils
