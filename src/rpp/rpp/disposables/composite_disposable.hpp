//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/disposables/interface_disposable.hpp>

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>

namespace rpp
{
class composite_disposable final : public interface_disposable
{
public:
    composite_disposable()
        : m_state{std::make_shared<state>()} {}

    composite_disposable(const composite_disposable& other) = default;
    composite_disposable(composite_disposable&& other) noexcept = default;

    bool is_disposed() const override
    {
        return !m_state || m_state->is_disposed();
    }

    void dispose() override
    {
        if (is_disposed())
            return;

       m_state->dispose();
    }

    void add(composite_disposable other) const
    {
        if (this == &other || other.is_disposed())
            return;

        if (is_disposed())
        {
            other.dispose();
            return;
        }

        m_state->add(std::make_shared<composite_disposable>(std::move(other)));
    }

    static composite_disposable empty()
    {
        return composite_disposable{empty_t{}};
    }

private:
    struct state
    {
        enum class State : uint8_t
        {
            None,    //< default state
            Edit,    //< set it during adding new element into deps or removing. After success -> back to None
            Disposed //< permanent state after dispose
        };

        bool is_disposed() const
        {
            return m_current_state.load(std::memory_order_acq_rel) == State::Disposed;
        }

        void dispose()
        {
            while (true)
            {
                State expected{State::None};
                if (m_current_state.compare_exchange_strong(expected, State::Disposed, std::memory_order::acq_rel))
                {
                    std::for_each(m_disposables.cbegin(), m_disposables.cend(), std::mem_fn(&interface_disposable::dispose));
                    m_disposables.clear();
                    return;
                }
            }
        }

        void add(std::shared_ptr<interface_disposable> disposable)
        {
            while (true)
            {
                State expected{State::None};
                if (m_current_state.compare_exchange_strong(expected, State::Edit, std::memory_order::acq_rel))
                {
                    m_disposables.push_back(std::move(disposable));
                    
                    m_current_state.store(State::None, std::memory_order::release);
                    return;
                }

                if (expected == State::Disposed)
                {
                    disposable->dispose();
                    return;
                }
            }
        }

    private:
        std::atomic<State>                                 m_current_state{};
        std::vector<std::shared_ptr<interface_disposable>> m_disposables{};
    };

private:
    struct empty_t {};

    composite_disposable(empty_t)
        : m_state{} {}

    std::shared_ptr<state> m_state{};
};
} // namespace rpp
