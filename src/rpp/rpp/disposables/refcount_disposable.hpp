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
#include <rpp/disposables/disposable_wrapper.hpp>

#include <atomic>

namespace rpp
{
class refcount_disposable final : public interface_disposable
{
public:
    refcount_disposable(disposable_ptr target)
        : refcount_disposable{disposable_wrapper{std::move(target)}}
    {}

    refcount_disposable(disposable_wrapper target)
        : m_target{std::move(target)}
    {}

    refcount_disposable(const refcount_disposable&)     = delete;
    refcount_disposable(refcount_disposable&&) noexcept = delete;

    bool is_disposed() const noexcept final { return m_refcount.load(std::memory_order_acquire) == 0; }

    void dispose() final
    {
        while (auto current_value = m_refcount.load(std::memory_order_acquire))
        {
            if (!m_refcount.compare_exchange_strong(current_value, current_value - 1, std::memory_order_acq_rel))
                continue;

            // was last one
            if (current_value == 1)
                m_target.dispose();

            return;
        }
    }

    void add_ref()
    {
        while (auto current_value = m_refcount.load(std::memory_order_acquire))
        {
            if (!m_refcount.compare_exchange_strong(current_value, current_value + 1, std::memory_order_acq_rel))
                continue;

            return;
        }
    }

private:
    disposable_wrapper m_target;
    std::atomic_size_t m_refcount{1};
};
} // namespace rpp