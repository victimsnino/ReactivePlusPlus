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
#include <rpp/disposables/composite_disposable.hpp>

#include <atomic>

namespace rpp
{
/**
 * @brief Disposable with counter inside. Each `add_ref` increments counter, while each `dispose()` call decrements. In case of reaching zero disposes underlying disposables
 * @warn Don't use it as disposable of observer due to `is_disposed()` would be false till counter reaches zero, so, observer can be also not `is_disposed()` during this time.
 * 
 * @ingroup disposables
 */
class refcount_disposable final : public interface_disposable
{
public:
    refcount_disposable() = default;
    refcount_disposable(disposable_ptr target) { m_underlying.add(std::move(target)); }

    refcount_disposable(const refcount_disposable&)     = delete;
    refcount_disposable(refcount_disposable&&) noexcept = delete;

    bool is_disposed() const noexcept override { return m_refcount.load(std::memory_order_acquire) == 0; }

    void dispose() override
    {
        while (auto current_value = m_refcount.load(std::memory_order_acquire))
        {
            if (!m_refcount.compare_exchange_strong(current_value, current_value - 1, std::memory_order_acq_rel))
                continue;

            // was last one
            if (current_value == 1)
                m_underlying.dispose();

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

    void add(disposable_ptr disposable)
    {
        m_underlying.add(std::move(disposable));
    }

private:
    composite_disposable m_underlying;
    std::atomic_size_t   m_refcount{1};
};
} // namespace rpp
