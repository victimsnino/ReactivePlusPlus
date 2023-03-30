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

namespace rpp
{
class composite_disposable final
{
public:
    composite_disposable()
        : m_state{std::make_shared<state>()} {}

    composite_disposable(const composite_disposable& other)                = default;
    composite_disposable(composite_disposable&& other) noexcept            = default;
    composite_disposable& operator=(const composite_disposable& other)     = default;
    composite_disposable& operator=(composite_disposable&& other) noexcept = default;

    bool is_disposed() const noexcept
    {
        return !m_state || m_state->is_disposed();
    }

    void dispose() const
    {
        if (m_state)
            m_state->dispose();
    }

    void add(const composite_disposable& other) const
    {
        if (m_state == other.m_state || other.is_disposed())
            return;

        if (m_state)
            m_state->add(other);
        else
            other.dispose();
    }

    bool is_empty() const noexcept
    {
        return !m_state;
    }

    static composite_disposable empty()
    {
        return composite_disposable{empty_t{}};
    }

private:
    class state
    {
    public:
        bool is_disposed() const noexcept
        {
            return m_current_state.load(std::memory_order_acquire) == State::Disposed;
        }

        void dispose()
        {
            while (true)
            {
                State expected{State::None};
                if (m_current_state.compare_exchange_strong(expected, State::Disposed, std::memory_order::acq_rel))
                {
                    for (const auto& variant : m_disposables)
                        std::visit([](const auto& d) { return d.dispose(); }, variant);

                    m_disposables.clear();
                    return;
                }

                if (expected == State::Disposed)
                    return;
            }
        }

        template<typename T>
        void add(T&& disposable)
        {
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

    private:
        enum class State : uint8_t
        {
            None,    // default state
            Edit,    // set it during adding new element into deps or removing. After success -> back to None
            Disposed // permanent state after dispose
        };

    private:
        std::atomic<State>                              m_current_state{};
        std::vector<std::variant<composite_disposable>> m_disposables{};
    };

    struct empty_t {};

    explicit composite_disposable(const empty_t) {}

    std::shared_ptr<state> m_state{};
};
} // namespace rpp
