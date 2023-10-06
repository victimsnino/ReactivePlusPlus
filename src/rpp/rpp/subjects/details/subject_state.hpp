//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/disposables/callback_disposable.hpp>
#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/observers/dynamic_observer.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/utils/utils.hpp>

#include <algorithm>
#include <memory>
#include <mutex>
#include <variant>
#include <vector>

namespace rpp::subjects::details
{
struct completed
{
};

struct disposed
{
};

template<rpp::constraint::decayed_type Type>
class subject_state final : public std::enable_shared_from_this<subject_state<Type>>
                          , public composite_disposable
{
    struct observers_t
    {
        std::vector<rpp::dynamic_observer<Type>> observers{};
        std::vector<rpp::dynamic_observer<Type>> observers_to_add{};
    };

    using state_t = std::variant<observers_t, std::exception_ptr, completed, disposed>;

public:
    template<rpp::constraint::observer_of_type<Type> TObs>
    void on_subscribe(TObs&& observer)
    {
        std::unique_lock lock{m_mutex};
        process_state_unsafe(
            m_state,
            [&](observers_t& observers) {
                auto observer_as_dynamic = std::forward<TObs>(observer).as_dynamic();
                if (m_state_flag & EState::Emitting || !observers.observers_to_add.empty())
                    observers.observers_to_add.push_back(observer_as_dynamic);
                else
                    observers.observers.push_back(observer_as_dynamic);

                lock.unlock();
                set_upstream(observer_as_dynamic);
            },
            [&](const std::exception_ptr& err) {
                lock.unlock();
                observer.on_error(err);
            },
            [&](completed) {
                lock.unlock();
                observer.on_completed();
            });
    }

    void on_next(const Type& v)
    {
        std::lock_guard lock{m_mutex};

        if (auto* observers = extract_observers_under_lock_if_there_unsafe())
        {
            m_state_flag |= EState::Emitting;
            rpp::utils::for_each(observers->observers, [&](const auto& sub) { sub.on_next(v); });
            if (std::exchange(m_state_flag, EState::None) & EState::NeedToCleanup)
                cleanup_observers(observers);
        }
    }

    void on_error(const std::exception_ptr& err)
    {
        std::lock_guard lock{m_mutex};
        rpp::utils::for_each(exchange_observers_under_lock_if_there_unsafe(err), [&](const auto& sub) { sub.on_error(err); });
    }

    void on_completed()
    {
        std::lock_guard lock{m_mutex};
        rpp::utils::for_each(exchange_observers_under_lock_if_there_unsafe(completed{}), rpp::utils::static_mem_fn<&dynamic_observer<Type>::on_completed>{});
    }

private:
    void dispose_impl() noexcept override
    {
        std::lock_guard lock{m_mutex};
        exchange_observers_under_lock_if_there_unsafe(disposed{});
    }

    static void cleanup_observers(observers_t* observers)
    {
        if (!observers)
            return;
        
        for (auto* target : {&observers->observers, &observers->observers_to_add})
            target->erase(std::remove_if(target->begin(), target->end(), rpp::utils::static_mem_fn<&rpp::dynamic_observer<Type>::is_disposed>()), target->end());
    }

    void set_upstream(rpp::dynamic_observer<Type>& obs)
    {
        obs.set_upstream(rpp::disposable_wrapper{make_callback_disposable(
            [weak = this->weak_from_this()]() noexcept // NOLINT(bugprone-exception-escape)
            {
                if (const auto shared = weak.lock())
                {
                    std::unique_lock lock{shared->m_mutex};
                    if (shared->m_state_flag & EState::Emitting)
                        shared->m_state_flag |= EState::NeedToCleanup;
                    else 
                        cleanup_observers(std::get_if<observers_t>(&shared->m_state));
                }
            })});
    }

    static void process_state_unsafe(state_t& state, const auto&... actions)
    {
        std::visit(rpp::utils::overloaded{actions..., rpp::utils::empty_function_any_t{}}, state);
    }

    observers_t* extract_observers_under_lock_if_there_unsafe()
    {
        if (auto obs = std::get_if<observers_t>(&m_state))
        {
            std::move(obs->observers_to_add.begin(), obs->observers_to_add.begin(), std::back_inserter(obs->observers));
            obs->observers_to_add.clear();
            return obs;
        }

        return {};
    }

    std::vector<rpp::dynamic_observer<Type>> exchange_observers_under_lock_if_there_unsafe(state_t&& new_val)
    {
        if (auto obs = extract_observers_under_lock_if_there_unsafe())
        {
            auto res = std::move(obs->observers);
            m_state = std::move(new_val);
            return res;
        }
        
        return {};
    }

private:
    enum EState : uint8_t
    {
        None          = 0b00,
        Emitting      = 0b01,
        NeedToCleanup = 0b10
    };

    state_t              m_state{};
    std::recursive_mutex m_mutex{};
    uint8_t              m_state_flag{};
};
} // namespace rpp::subjects::details