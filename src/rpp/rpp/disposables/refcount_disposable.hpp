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
struct refocunt_disposable_state_t
{
    composite_disposable underlying;
    std::atomic_size_t   refcount{1};

    void dispose()
    {
        // just need atomicity, not guarding anything
        if (refcount.fetch_sub(1) == 1)
            underlying.dispose();
    }
};

class inner_refcount_disposable final : public details::base_composite_disposable
{
public:
    inner_refcount_disposable(const std::shared_ptr<refocunt_disposable_state_t>& state)
        : m_state{state}
    {
    }

    inner_refcount_disposable(const inner_refcount_disposable&)     = delete;
    inner_refcount_disposable(inner_refcount_disposable&&) noexcept = delete;

    void dispose_impl() noexcept final
    {
        m_state->dispose();
    }

    using interface_composite_disposable::add;

    void add(disposable_wrapper disposable) final
    {
        m_state->underlying.add(std::move(disposable));
    }

private:
    std::shared_ptr<refocunt_disposable_state_t> m_state;
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
class refcount_disposable : public details::base_composite_disposable
                          , public std::enable_shared_from_this<refcount_disposable>
{

public:
    refcount_disposable() = default;

    refcount_disposable(disposable_wrapper target) { m_state.underlying.add(std::move(target)); }

    refcount_disposable(const refcount_disposable&)     = delete;
    refcount_disposable(refcount_disposable&&) noexcept = delete;

    bool is_disposed_underlying() const noexcept
    {
        // just need atomicity, not guarding anything
        return m_state.refcount.load() == 0;
    }

    void dispose_impl() noexcept final
    {
        m_state.dispose();
    }

    composite_disposable_wrapper add_ref()
    {
        auto current_value = m_state.refcount.load();

        // just need atomicity, not guarding anything
        while (current_value && !m_state.refcount.compare_exchange_strong(current_value, current_value + 1)){};

        if (!current_value)
            return composite_disposable_wrapper{};

        return composite_disposable_wrapper{std::make_shared<details::inner_refcount_disposable>(std::shared_ptr<details::refocunt_disposable_state_t>{this->shared_from_this(), &this->m_state})};
    }

    using interface_composite_disposable::add;

    void add(disposable_wrapper disposable) final
    {
        m_state.underlying.add(std::move(disposable));
    }

private:
    details::refocunt_disposable_state_t m_state{};
};
} // namespace rpp
