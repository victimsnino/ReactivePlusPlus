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

#include <rpp/defs.hpp>
#include <rpp/observers/details/fwd.hpp>

#include <rpp/disposables/disposable_wrapper.hpp>

#include <atomic>
#include <vector>

namespace rpp::details::observers
{
class atomic_bool
{
public:
    atomic_bool() = default;
    atomic_bool(atomic_bool&& other) noexcept
        // just need atomicity, not guarding anything
        : m_value{other.m_value.load(std::memory_order::relaxed)}
    {}

    bool test() const noexcept 
    {
        // just need atomicity, not guarding anything
        return m_value.load(std::memory_order::relaxed);
    }

    void set() noexcept
    {
        // just need atomicity, not guarding anything
        m_value.store(true, std::memory_order::relaxed);
    }
private:
    std::atomic_bool m_value{};
};

class non_atomic_bool
{
public:
    non_atomic_bool() = default;
    non_atomic_bool(atomic_bool&& other) noexcept = default;

    bool test() const noexcept 
    {
        return m_value;
    }

    void set() noexcept
    {
        m_value = true;
    }
private:
    bool m_value{};
};

template<typename DisposableContainer, typename Bool>
class local_disposable_strategy
{
public:
    local_disposable_strategy() = default;
    local_disposable_strategy(local_disposable_strategy&& other) noexcept = default;

    void add(const disposable_wrapper& d)
    {
        m_upstreams.push_back(d);
    }

    bool is_disposed() const noexcept
    {
        // just need atomicity, not guarding anything
        return m_is_disposed.test();
    }

    void dispose() const
    {
        // just need atomicity, not guarding anything
        m_is_disposed.set();
        m_upstreams.dispose();
    }

private:
    RPP_NO_UNIQUE_ADDRESS DisposableContainer m_upstreams{};
    mutable Bool                              m_is_disposed{};
};

struct none_disposable_strategy
{
    static void add(const rpp::disposable_wrapper&) {}

    static bool is_disposed() noexcept { return false; }

    static void dispose() {}
};

}