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

#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/disposables/details/base_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>

#include <atomic>
#include <memory>
#include <limits>

namespace rpp
{
class refcount_disposable : public std::enable_shared_from_this<refcount_disposable>
                          , public rpp::composite_disposable 
{
    class refocunt_disposable_inner final : public rpp::composite_disposable, public std::enable_shared_from_this<refocunt_disposable_inner>
    {
    public:
        refocunt_disposable_inner(std::shared_ptr<refcount_disposable> state)
            : m_state{std::move(state)} {}
            
        void dispose_impl() noexcept override
        {
            m_state->remove(rpp::disposable_wrapper::from_shared(shared_from_this()));
            m_state->release();
            m_state.reset();
        }

    private:
        std::shared_ptr<refcount_disposable> m_state;
    };

    void release()
    {
        auto current_value = m_refcount.load(std::memory_order::seq_cst);
        while (current_value != s_disposed)
        {
            const size_t new_value = current_value == 1 ? s_disposed : current_value - 1;
            if (!m_refcount.compare_exchange_strong(current_value, new_value, std::memory_order::seq_cst))
                continue;

            if (new_value == s_disposed)
                dispose();
            return;
        }
    }

    void dispose_impl() noexcept override
    {
        m_refcount.store(s_disposed, std::memory_order::seq_cst);
    }

public:
    refcount_disposable() = default;

    composite_disposable_wrapper add_ref()
    {
        auto current_value = m_refcount.load(std::memory_order::seq_cst);
        while (true)
        {
            if (current_value == s_disposed)
                return {};

            // just need atomicity, not guarding anything
            if (m_refcount.compare_exchange_strong(current_value, current_value + 1, std::memory_order::seq_cst))
            {
                auto inner = std::make_shared<refocunt_disposable_inner>(shared_from_this());
                add(rpp::disposable_wrapper::from_weak(inner));
                return composite_disposable_wrapper{inner};
            }
        }
    }

private:
    std::atomic<size_t>     m_refcount{0};
    constexpr static size_t s_disposed = std::numeric_limits<size_t>::max();
};
} // namespace rpp
