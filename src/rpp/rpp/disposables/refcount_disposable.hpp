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

namespace rpp::details
{
struct refocunt_disposable_state_t final : public composite_disposable 
{
    refocunt_disposable_state_t() = default;
    refocunt_disposable_state_t(const refocunt_disposable_state_t&) = delete;
    refocunt_disposable_state_t(refocunt_disposable_state_t&&) noexcept = delete;

private:
    using composite_disposable::dispose;

    void dispose_impl() noexcept override
    {
        m_refcount.store(0, std::memory_order::relaxed);
    }

public:
    void try_dispose()
    {
        // just need atomicity, not guarding anything
        if (m_refcount.fetch_sub(1, std::memory_order::relaxed) == 1)
            dispose();
    }

    bool is_disposed_by_any() const noexcept
    {
        return m_refcount.load(std::memory_order::relaxed) == 0 || is_disposed();
    }

    bool add_ref() 
    {
        auto current_value = m_refcount.load(std::memory_order::relaxed);
        // just need atomicity, not guarding anything
        while (current_value && !m_refcount.compare_exchange_strong(current_value, current_value + 1, std::memory_order::relaxed, std::memory_order::relaxed)){};

        return current_value;
    }

private:
    std::atomic_size_t m_refcount{1};
};

class inner_refcount_disposable final : public interface_composite_disposable
{
public:
    explicit inner_refcount_disposable(const std::shared_ptr<refocunt_disposable_state_t>& state)
        : m_state{state}
    {
    }

    inner_refcount_disposable(const inner_refcount_disposable&)     = delete;
    inner_refcount_disposable(inner_refcount_disposable&&) noexcept = delete;

    void dispose() noexcept override
    {
        if (m_disposed.exchange(true, std::memory_order::relaxed) == false)
            m_state->try_dispose();
    }

    using interface_composite_disposable::add;

    void add(disposable_wrapper disposable) override
    {
        m_state->add(std::move(disposable));
    }

    bool is_disposed() const noexcept override
    {
        return m_disposed.load(std::memory_order::relaxed) || m_state->is_disposed_by_any();
    } 

private:
    std::shared_ptr<refocunt_disposable_state_t> m_state;
    std::atomic_bool                     m_disposed{};
};
}

namespace rpp
{
/**
 * @brief Disposable with counter inside. Each `add_ref` increments counter, while each `dispose()` call decrements. In case of reaching zero disposes underlying disposables
 * @warning Don't use it as disposable of observer due to `is_disposed()` would be false till counter reaches zero, so, observer can be also not `is_disposed()` during this time.
 *
 * @ingroup disposables
 */
class refcount_disposable : public interface_composite_disposable,
                            public std::enable_shared_from_this<refcount_disposable> {

public:
    refcount_disposable() = default;

    refcount_disposable(const refcount_disposable&)     = delete;
    refcount_disposable(refcount_disposable&&) noexcept = delete;

    bool is_disposed_underlying() const noexcept
    {
        return m_state.is_disposed_by_any();
    }

    composite_disposable_wrapper add_ref()
    {
        if (m_state.add_ref())
            return composite_disposable_wrapper{std::make_shared<details::inner_refcount_disposable>(std::shared_ptr<details::refocunt_disposable_state_t>{this->shared_from_this(), &this->m_state})};

        return composite_disposable_wrapper{};
    }

    composite_disposable_wrapper get_underlying()
    {
        return composite_disposable_wrapper::from_shared(std::shared_ptr<rpp::composite_disposable>{this->shared_from_this(), &this->m_state});
    }

    bool is_disposed() const noexcept final
    {
        return m_disposed.load(std::memory_order::relaxed) || m_state.is_disposed_by_any();
    } 

    void dispose() noexcept final
    {
        if (m_disposed.exchange(true, std::memory_order::relaxed) == false)
            m_state.try_dispose();
    }

    using interface_composite_disposable::add;

    void add(disposable_wrapper disposable) final
    {
        m_state.add(std::move(disposable));
    }

private:
    details::refocunt_disposable_state_t m_state{};
    std::atomic_bool                     m_disposed{};
};
} // namespace rpp
