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

#include <rpp/observers/details/fwd.hpp>

#include <rpp/disposables/disposable_wrapper.hpp>

#include <atomic>
#include <vector>

namespace rpp::details::observers
{

class local_disposable_strategy
{
public:
    local_disposable_strategy() = default;
    local_disposable_strategy(local_disposable_strategy&& other) noexcept
        : m_upstreams(std::move(other.m_upstreams))
        // just need atomicity, not guarding anything
        , m_is_disposed(other.m_is_disposed.load(std::memory_order::relaxed))
    {}

    void add(const disposable_wrapper& d)
    {
        m_upstreams.push_back(d);
    }

    bool is_disposed() const noexcept
    {
        // just need atomicity, not guarding anything
        return m_is_disposed.load(std::memory_order::relaxed);
    }

    void dispose() const
    {
        // just need atomicity, not guarding anything
        m_is_disposed.store(true, std::memory_order::relaxed);
        for (const auto& d : m_upstreams)
            d.dispose();
    }

private:
    std::vector<disposable_wrapper> m_upstreams{};
    mutable std::atomic_bool        m_is_disposed{false};
};

struct none_disposable_strategy
{
    static void add(const rpp::disposable_wrapper&) {}

    static bool is_disposed() noexcept { return false; }

    static void dispose() {}
};

}