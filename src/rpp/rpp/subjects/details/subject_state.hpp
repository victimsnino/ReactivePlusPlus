//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/utils/constraints.hpp>
#include <rpp/observers/dynamic_observer.hpp>
#include <rpp/utils/utils.hpp>

#include <memory>
#include <mutex>
#include <vector>
#include <variant>

namespace rpp::subjects::details
{
struct completed{};

template<rpp::constraint::decayed_type Type>
class subject_state : public std::enable_shared_from_this<subject_state<Type>>
{
    using shared_observers = std::shared_ptr<std::vector<rpp::dynamic_observer<Type>>>;
    using state_t = std::variant<shared_observers, std::exception_ptr, completed>;

public:
    void on_subscribe(const rpp::dynamic_observer<Type>& observer) 
    {
        std::unique_lock lock{m_mutex};
        process_state(
            m_state,
            [&](const shared_observers& observers)
            {
                auto new_observers = std::make_shared<std::vector<rpp::dynamic_observer<Type>>>();
                if (observers)
                {
                    new_observers->reserve(observers->size() + 1);
                    std::copy_if(observers->cbegin(),
                                 observers->cend(),
                                 std::back_inserter(*observers),
                                 [](const rpp::dynamic_observer<Type>& obs) { return !obs.is_disposed(); });
                }
                new_observers->push_back(observer);
                m_state = new_observers;

                lock.unlock();

                // add_callback_on_unobserverscribe(observer);
            },
            [&](std::exception_ptr err)
            {
                lock.unlock();
                observer.on_error(err);
            },
            [&](completed)
            {
                lock.unlock();
                observer.on_completed();
            });
    }

    void on_next(const Type& v)
    {
        if (const auto observers = extract_observers_under_lock_if_there())
            rpp::utils::for_each(*observers, [&](const auto& sub) { sub.on_next(v); });
    }

    void on_error(const std::exception_ptr& err)
    {
        if (const auto observers = exchange_observers_under_lock_if_there(state_t{err}))
            rpp::utils::for_each(*observers, [&](const auto& sub) { sub.on_error(err); });
    }

    void on_completed()
    {
        if (const auto observers = exchange_observers_under_lock_if_there(completed{}))
            rpp::utils::for_each(*observers, utils::static_mem_fn<&dynamic_observer<Type>::on_completed>{});
    }

private:
    static void process_state(const state_t& state, const auto&...actions)
    {
        std::visit(rpp::utils::overloaded{ actions..., utils::empty_function_any_t{} }, state);
    }

    shared_observers extract_observers_under_lock_if_there()
    {
        std::unique_lock lock{ m_mutex };

        if (!std::holds_alternative<shared_observers>(m_state))
            return {};

        const auto observers = std::get<shared_observers>(m_state);

        lock.unlock();
        return observers;
    }

    shared_observers exchange_observers_under_lock_if_there(state_t&& new_val)
    {
        std::unique_lock lock{ m_mutex };

        if (!std::holds_alternative<shared_observers>(m_state))
            return {};

        const auto observers = std::get<shared_observers>(std::exchange(m_state, std::move(new_val)));

        lock.unlock();
        return observers;
    }

private:
    state_t    m_state{};
    std::mutex m_mutex{};
};
}