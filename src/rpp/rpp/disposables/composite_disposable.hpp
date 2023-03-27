//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/disposables/fwd.hpp>

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace rpp::details
{
class variant_disposable;

class composite_disposable_state
{
public:
    bool is_disposed() const noexcept
    {
        return m_current_state.load(std::memory_order_acquire) == State::Disposed;
    }

    void dispose();

    template<typename T>
    void add(T&& disposable);

private:
    enum class State : uint8_t
    {
        None,
        //< default state
        Edit,
        //< set it during adding new element into deps or removing. After success -> back to None
        Disposed //< permanent state after dispose
    };

private:
    std::atomic<State>              m_current_state{};
    std::vector<variant_disposable> m_disposables{};
};
} // namespace rpp::details

namespace rpp
{
class composite_disposable final
{
public:
    composite_disposable()
        : m_state{std::make_shared<details::composite_disposable_state>()} {}

    composite_disposable(const composite_disposable& other)                = default;
    composite_disposable(composite_disposable&& other) noexcept            = default;
    composite_disposable& operator=(const composite_disposable& other)     = default;
    composite_disposable& operator=(composite_disposable&& other) noexcept = default;

    [[nodiscard]] bool is_disposed() const noexcept
    {
        return !m_state || m_state->is_disposed();
    }

    void dispose() const noexcept
    {
        if (is_disposed())
            return;

       m_state->dispose();
    }

    void add(const composite_disposable& other) const
    {
        if (m_state == other.m_state || other.is_disposed())
            return;

        if (is_disposed())
        {
            other.dispose();
            return;
        }

        m_state->add(other);
    }

    static composite_disposable empty()
    {
        return composite_disposable{empty_t{}};
    }

private:
    struct empty_t {};

    composite_disposable(empty_t)
        : m_state{} {}

    std::shared_ptr<details::composite_disposable_state> m_state{};
};
} // namespace rpp

namespace rpp::details
{
class variant_disposable
{
public:
    using variant = std::variant<composite_disposable>;

    variant_disposable(variant disposable)
        : m_variant{std::move(disposable)} {}

    variant_disposable(const variant_disposable&)     = default;
    variant_disposable(variant_disposable&&) noexcept = default;

    [[nodiscard]] bool is_disposed() const noexcept
    {
        return std::visit([](const auto& d) { return d.is_disposed(); }, m_variant);
    }

    void dispose() const noexcept
    {
        return std::visit([](const auto& d) { return d.dispose(); }, m_variant);
    }

private:
    std::variant<composite_disposable> m_variant;
};

inline void composite_disposable_state::dispose() {
    while (true)
    {
        State expected{State::None};
        if (m_current_state.compare_exchange_strong(expected, State::Disposed, std::memory_order::acq_rel))
        {
            std::for_each(m_disposables.cbegin(), m_disposables.cend(), std::mem_fn(&details::variant_disposable::dispose));
            m_disposables.clear();
            return;
        }
    }
}

template<typename T>
inline void composite_disposable_state::add(T&& disposable) {
    while (true)
    {
        State expected{State::None};
        if (m_current_state.compare_exchange_strong(expected, State::Edit, std::memory_order::acq_rel))
        {
            m_disposables.emplace_back(std::forward<T>(disposable));

            m_current_state.store(State::None, std::memory_order::release);
            return;
        }

        if (expected == State::Disposed)
        {
            disposable.dispose();
            return;
        }
    }
}
} // namespace rpp::details