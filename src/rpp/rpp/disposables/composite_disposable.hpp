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
#include <rpp/disposables/interface_composite_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>

#include <atomic>
#include <memory>
#include <vector>

namespace rpp
{
/**
 * @brief Disposable which can keep some other sub-disposables. When this root disposable is disposed, then all sub-disposables would be disposed too.
 * 
 * @ingroup disposables
 */
class composite_disposable : public interface_composite_disposable
{
public:
    composite_disposable() = default;

    composite_disposable(const composite_disposable&) = delete;
    composite_disposable(composite_disposable&& other) noexcept = delete;
    
    bool is_disposed() const noexcept final
    {
        return m_current_state.load(std::memory_order_acquire) == State::Disposed;
    }

    void dispose() noexcept final
    {
        while (true)
        {
            State expected{State::None};
            if (m_current_state.compare_exchange_strong(expected, State::Disposed, std::memory_order::acq_rel))
            {
                dispose_impl();

                for (const auto& d : m_disposables)
                    d.dispose();

                m_disposables.clear();
                return;
            }

            if (expected == State::Disposed)
                return;
        }
    }

    using interface_composite_disposable::add;
    
    void add(disposable_wrapper disposable) override
    {
        if (disposable.is_disposed() || disposable.get_original().get() == this)
            return;

        while (true)
        {
            State expected{State::None};
            if (m_current_state.compare_exchange_strong(expected, State::Edit, std::memory_order::acq_rel))
            {
                m_disposables.emplace_back(std::move(disposable));
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

protected:
    virtual void dispose_impl() noexcept {}

private:
    enum class State : uint8_t
    {
        None,    // default state
        Edit,    // set it during adding new element into deps or removing. After success -> back to None
        Disposed // permanent state after dispose
    };

    std::vector<disposable_wrapper> m_disposables{};
    std::atomic<State>              m_current_state{};
};
} // namespace rpp
