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

#include <rpp/disposables/fwd.hpp>
#include <rpp/disposables/interface_disposable.hpp>

#include <atomic>

namespace rpp::details
{
class base_disposable : public interface_disposable
{
public:
    base_disposable()                           = default;
    base_disposable(const base_disposable&)     = delete;
    base_disposable(base_disposable&&) noexcept = delete;

    bool is_disposed() const noexcept final { return m_disposed.load(std::memory_order_acquire); }

    void dispose() final
    {
        if (m_disposed.exchange(true, std::memory_order_acq_rel) == false)
            dispose_impl();
    }

protected:
    virtual void dispose_impl() = 0;

private:
    std::atomic_bool m_disposed{};
};
}