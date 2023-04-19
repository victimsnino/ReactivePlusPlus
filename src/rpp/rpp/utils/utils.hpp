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

#include <rpp/defs.hpp>
#include <rpp/utils/constraints.hpp>

#include <concepts>
#include <iterator>

namespace rpp::utils {

template<constraint::iterable T>
using iterable_value_t = std::iter_value_t<decltype(std::begin(std::declval<T>()))>;

struct none_lock
{
    static void lock() {}
    static void unlock() {}
};

/**
 * @brief Calls passed function during destruction
 */
template<std::invocable Fn>
class finally_action
{
public:
    finally_action(Fn&& fn)
        : m_fn{std::move(fn)} {}

    finally_action(const Fn& fn)
        : m_fn{fn} {}

    ~finally_action() noexcept
    {
        m_fn();
    }
private:
    RPP_NO_UNIQUE_ADDRESS Fn m_fn;
};
}