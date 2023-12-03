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

namespace rpp::details
{
class refocunt_disposable_state_t final : public rpp::composite_disposable 
{
public:
    void dispose_impl() noexcept override
    {
        m_refcount.store(s_disposed, std::memory_order::relaxed);
    }

    void release()
    {
        auto current_value = m_refcount.load(std::memory_order::relaxed);
        while (true)
        {
            if (current_value == s_disposed)
                return;

            const size_t new_value = current_value == 1 ? s_disposed : current_value - 1;
            if (!m_refcount.compare_exchange_strong(current_value, new_value, std::memory_order::relaxed, std::memory_order::relaxed))
                continue;

            if (new_value == s_disposed)
                dispose();
            return;
        }
    }

    bool add_ref() 
    {
        auto current_value = m_refcount.load(std::memory_order::relaxed);
        // just need atomicity, not guarding anything
        while (current_value != s_disposed && !m_refcount.compare_exchange_strong(current_value, current_value + 1, std::memory_order::relaxed, std::memory_order::relaxed)){};

        return current_value != s_disposed;
    }

private:
    std::atomic<size_t>     m_refcount{0};
    constexpr static size_t s_disposed = std::numeric_limits<size_t>::max();
};

class refocunt_disposable_inner final : public rpp::composite_disposable, public std::enable_shared_from_this<refocunt_disposable_inner>
{
public:
    refocunt_disposable_inner(const std::shared_ptr<refocunt_disposable_state_t>& state)
        : m_state{state} {}
        
    void dispose_impl() noexcept override
    {
        m_state->remove(rpp::disposable_wrapper::from_weak(weak_from_this()));
        m_state->release();
        m_state.reset();
    }

private:
    std::shared_ptr<refocunt_disposable_state_t> m_state;
};
}

namespace rpp
{
class refcount_disposable : public std::enable_shared_from_this<refcount_disposable> {

public:
    refcount_disposable() = default;

    bool is_disposed_underlying() const noexcept
    {
        return m_state.is_disposed();
    }

    composite_disposable_wrapper add_ref()
    {
        if (m_state.add_ref())
        {
            auto inner   = std::make_shared<details::refocunt_disposable_inner>(std::shared_ptr<details::refocunt_disposable_state_t>{this->shared_from_this(), &this->m_state});
            m_state.add(rpp::disposable_wrapper::from_weak(inner));
            return composite_disposable_wrapper{inner};
        }

        return composite_disposable_wrapper{};
    }

    composite_disposable_wrapper get_underlying()
    {
        return composite_disposable_wrapper::from_shared(std::shared_ptr<rpp::composite_disposable>{this->shared_from_this(), &this->m_state});
    }

private:
    details::refocunt_disposable_state_t m_state{};
};
} // namespace rpp
