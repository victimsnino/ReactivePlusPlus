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
#include <mutex>
#include <thread>

namespace rpp::utils {

template<constraint::iterable T>
using iterable_value_t = std::iter_value_t<decltype(std::begin(std::declval<T>()))>;

struct none_lock
{
    static void lock() {}
    static void unlock() {}
};

struct none_condition_variable
{
    static void notify_one() {}
    static void wait(std::unique_lock<none_lock>&, const auto&) {}

    template<typename _Clock, typename _Duration>
    static bool wait_until(std::unique_lock<none_lock>&, const std::chrono::time_point<_Clock, _Duration>& time, const auto& predicate)
    {
        std::this_thread::sleep_until(time);
        return predicate();
    }
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