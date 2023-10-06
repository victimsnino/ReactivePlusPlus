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
    using observers_t = std::vector<rpp::dynamic_observer<Type>>;
    using state_t   = std::variant<observers_t, std::exception_ptr, completed, disposed>;
    using shared_state_t   = std::shared_ptr<state_t>;
    using atomic_state_t   = rpp::utils::atomic_shared_ptr<state_t>;

public:
    template<rpp::constraint::observer_of_type<Type> TObs>
    void on_subscribe(TObs&& observer)
    {
        auto state = std::atomic_load_explicit(&m_state, std::memory_order_relaxed);
        while(true)
        {
            if (!state) {
                auto observer_as_dynamic = std::forward<TObs>(observer).as_dynamic();
                std::vector<rpp::dynamic_observer<Type>> res{observer_as_dynamic};
                if (state)
                    continue;
                
                if (!std::atomic_compare_exchange_strong_explicit(&m_state, &state, atomic_state_t{std::make_shared<state_t>(std::move(res))}, std::memory_order::relaxed, std::memory_order::relaxed)) {
                    continue;
                }
                set_upstream(observer_as_dynamic);
                return;
            }

            if (process_state_unsafe(
                state,
                [&](const observers_t&) {
                    auto observer_as_dynamic = std::forward<TObs>(observer).as_dynamic();
                    std::vector<rpp::dynamic_observer<Type>> res{};

                    while(true)
                    {
                        if (!std::holds_alternative<observers_t>(*state))
                            return false;
                        
                        const auto& observers = std::get<observers_t>(*state);

                        std::vector<rpp::dynamic_observer<Type>> res{};
                        res.reserve(observers.size() + 1);
                        std::copy_if(observers.cbegin(),
                                    observers.cend(),
                                    std::back_inserter(res),
                                    rpp::utils::static_not_mem_fn<&dynamic_observer<Type>::is_disposed>{});
                        res.push_back(observer_as_dynamic);
                        
                        if (!std::atomic_compare_exchange_strong_explicit(&m_state, &state, atomic_state_t{std::make_shared<state_t>(std::move(res))}, std::memory_order::relaxed, std::memory_order::relaxed)) {
                            res.clear();
                            continue;
                        }
                    }
                    set_upstream(observer_as_dynamic);
                    return true;
                },
                [&](const std::exception_ptr& err) {
                    observer.on_error(err);
                    return true;
                },
                [&](completed) {
                    observer.on_completed();
                    return true;
                },
                [](const auto&) {return true;}))
                    return;
        }
    }

    void on_next(const Type& v)
    {
        if (const auto obs = extract_observers_under_lock_if_there())
            rpp::utils::for_each(std::get<observers_t>(*obs), [&](const auto& sub) { sub.on_next(v); });
    }

    void on_error(const std::exception_ptr& err)
    {
        if (const auto obs = exchange_observers_under_lock_if_there(std::make_shared<state_t>(err)))
            rpp::utils::for_each(std::get<observers_t>(*obs), [&](const auto& sub) { sub.on_error(err); });
    }

    void on_completed()
    {
        if (const auto obs = exchange_observers_under_lock_if_there(std::make_shared<state_t>(completed{})))
            rpp::utils::for_each(std::get<observers_t>(*obs), rpp::utils::static_mem_fn<&dynamic_observer<Type>::on_completed>{});
    }

private:
    void dispose_impl() noexcept override
    {
        exchange_observers_under_lock_if_there(std::make_shared<state_t>(disposed{}));
    }

    void set_upstream(rpp::dynamic_observer<Type>& obs)
    {
        obs.set_upstream(rpp::disposable_wrapper{make_callback_disposable(
            [weak = this->weak_from_this()]() noexcept // NOLINT(bugprone-exception-escape)
            {
                if (const auto shared = weak.lock())
                {
                    auto state = std::atomic_load_explicit(&shared->m_state, std::memory_order::relaxed);
                    std::vector<rpp::dynamic_observer<Type>> res{};

                    while(true)
                    {
                        if (!std::holds_alternative<observers_t>(*state))
                            return;

                        const auto& observers = std::get<observers_t>(*state);
                        if (observers.empty())
                            return;
                        
                        res.reserve(observers.size() - 1);
                        std::copy_if(observers.cbegin(),
                                    observers.cend(),
                                    std::back_inserter(res),
                                    rpp::utils::static_not_mem_fn<&dynamic_observer<Type>::is_disposed>{});
                        
                        if (!std::atomic_compare_exchange_strong_explicit(&shared->m_state, &state, atomic_state_t{std::make_shared<state_t>(std::move(res))}, std::memory_order::relaxed, std::memory_order::relaxed)) {
                            res.clear();
                            continue;
                        }
                    }
                }
            })});
    }

    static auto process_state_unsafe(const shared_state_t& state, const auto&... actions)
    {
        return std::visit(rpp::utils::overloaded{actions..., rpp::utils::empty_function_any_t{}}, *state);
    }

    shared_state_t extract_observers_under_lock_if_there()
    {
        auto state = std::atomic_load_explicit(&m_state, std::memory_order_relaxed);

        if (!std::holds_alternative<observers_t>(*state))
            return {};

        return state;
    }

    shared_state_t exchange_observers_under_lock_if_there(atomic_state_t&& new_val)
    {
        auto state = std::atomic_load_explicit(&m_state, std::memory_order_relaxed);

        while (true)
        {
            if (!std::holds_alternative<observers_t>(*state))
                return {};

            if (std::atomic_compare_exchange_strong_explicit(&m_state, &state, atomic_state_t{std::move(new_val)}, std::memory_order::relaxed, std::memory_order::relaxed))
                return state;
        }
    }

private:
    atomic_state_t m_state{};
};
} // namespace rpp::subjects::details