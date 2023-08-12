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

#include <rpp/utils/constraints.hpp>
#include <concepts>
namespace rpp
{
struct interface_disposable;
struct interface_composite_disposable;

class composite_disposable;

template<std::invocable Fn>
class callback_disposable;

class refcount_disposable;

template<std::invocable Fn>
auto make_callback_disposable(Fn&& invocable);

template<rpp::constraint::decayed_type TDisposable>
class disposable_wrapper_impl;

using disposable_wrapper = disposable_wrapper_impl<interface_disposable>;
using composite_disposable_wrapper = disposable_wrapper_impl<interface_composite_disposable>;
} // namespace rpp