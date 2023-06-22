//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <concepts>

namespace rpp
{
class base_disposable;

class disposable_wrapper;

template<std::invocable Fn>
class callback_disposable;

template<std::invocable Fn>
auto make_callback_disposable(Fn&& invocable);
} // namespace rpp