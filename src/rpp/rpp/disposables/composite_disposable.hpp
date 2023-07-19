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

#include <atomic>
#include <memory>
#include <vector>

namespace rpp
{
class composite_disposable : public interface_disposable
{
public:
    composite_disposable() = default;

    composite_disposable(const composite_disposable&) = delete;
    composite_disposable(composite_disposable&&)      = delete;

    bool is_disposed() const noexcept final
    {
        return m_current_state.load(std::memory_order_acquire) == State::Disposed;
    }

    void dispose() final
    {
        while (true)
        {
            State expected{State::None};
            if (m_current_state.compare_exchange_strong(expected, State::Disposed, std::memory_order::acq_rel))
            {
                dispose_impl();

                for (const auto& d : m_disposables)
                    d->dispose();

                m_disposables.clear();
                return;
            }

            if (expected == State::Disposed)
                return;
        }
    }

    void add(std::shared_ptr<interface_disposable> disposable)
    {
        if (!disposable || disposable.get() == this || disposable->is_disposed())
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
                disposable->dispose();
                return;
            }
        }
    }

    template<std::invocable Fn>
    void add(Fn&& invocable)
    {
        add(std::make_shared<rpp::callback_disposable<std::decay_t<Fn>>>(std::forward<Fn>(invocable)));
    }

protected:
    virtual void dispose_impl() {}

private:
    enum class State
    {
        None,    // default state
        Edit,    // set it during adding new element into deps or removing. After success -> back to None
        Disposed // permanent state after dispose
    };

    std::atomic<State>                                 m_current_state{};
    std::vector<std::shared_ptr<interface_disposable>> m_disposables{};
};
} // namespace rpp
