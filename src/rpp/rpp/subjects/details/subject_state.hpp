//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/fwd.hpp>

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

    template<rpp::constraint::decayed_type Type, bool Serialized>
    class subject_state : public std::enable_shared_from_this<subject_state<Type, Serialized>>
        , public composite_disposable
    {
        using shared_observers = std::shared_ptr<std::vector<rpp::dynamic_observer<Type>>>;
        using state_t          = std::variant<shared_observers, std::exception_ptr, completed, disposed>;

    public:
        using expected_disposable_strategy = rpp::details::observables::atomic_fixed_disposable_strategy_selector<1>;

        template<rpp::constraint::observer_of_type<Type> TObs>
        void on_subscribe(TObs&& observer)
        {
            std::unique_lock lock{m_mutex};
            process_state_unsafe(
                m_state,
                [&](const shared_observers& observers) {
                    auto new_observers       = make_copy_of_subscribed_observers(true, observers);
                    auto observer_as_dynamic = std::forward<TObs>(observer).as_dynamic();
                    new_observers->push_back(observer_as_dynamic);
                    m_state = std::move(new_observers);

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
            std::lock_guard lock{m_serialized_mutex};
            if (const auto observers = extract_observers_under_lock_if_there())
                rpp::utils::for_each(*observers, [&](const auto& sub) { sub.on_next(v); });
        }

        void on_error(const std::exception_ptr& err)
        {
            {
                std::lock_guard lock{m_serialized_mutex};
                if (const auto observers = exchange_observers_under_lock_if_there(err))
                    rpp::utils::for_each(*observers, [&](const auto& sub) { sub.on_error(err); });
            }
            dispose();
        }

        void on_completed()
        {
            {
                std::lock_guard lock{m_serialized_mutex};
                if (const auto observers = exchange_observers_under_lock_if_there(completed{}))
                    rpp::utils::for_each(*observers, rpp::utils::static_mem_fn<&dynamic_observer<Type>::on_completed>{});
            }
            dispose();
        }

    private:
        void composite_dispose_impl(interface_disposable::Mode) noexcept override
        {
            exchange_observers_under_lock_if_there(disposed{});
        }

        void set_upstream(rpp::dynamic_observer<Type>& obs)
        {
            obs.set_upstream(rpp::disposable_wrapper{make_callback_disposable(
                [weak = this->weak_from_this()]() noexcept // NOLINT(bugprone-exception-escape)
                {
                    if (const auto shared = weak.lock())
                    {
                        std::unique_lock lock{shared->m_mutex};
                        process_state_unsafe(shared->m_state,
                                             [&](const shared_observers& observers) {
                                                 shared->m_state = make_copy_of_subscribed_observers(false, observers);
                                             });
                    }
                })});
        }

        static shared_observers make_copy_of_subscribed_observers(bool add, const shared_observers& current_subs)
        {
            auto subs = std::make_shared<std::vector<dynamic_observer<Type>>>();
            subs->reserve(deduce_new_size(add, current_subs));
            if (current_subs)
            {
                std::copy_if(current_subs->cbegin(),
                             current_subs->cend(),
                             std::back_inserter(*subs),
                             rpp::utils::static_not_mem_fn<&dynamic_observer<Type>::is_disposed>{});
            }
            return subs;
        }

        static size_t deduce_new_size(bool add, const shared_observers& current_subs)
        {
            if (!current_subs)
                return add ? 1 : 0;

            if (add)
                return current_subs->size() + 1;

            return std::max(current_subs->size(), size_t{1}) - 1;
        }

        static void process_state_unsafe(const state_t& state, const auto&... actions)
        {
            std::visit(rpp::utils::overloaded{actions..., rpp::utils::empty_function_any_t{}}, state);
        }

        shared_observers extract_observers_under_lock_if_there()
        {
            std::lock_guard lock{m_mutex};

            if (!std::holds_alternative<shared_observers>(m_state))
                return {};

            return std::get<shared_observers>(m_state);
        }

        shared_observers exchange_observers_under_lock_if_there(state_t&& new_val)
        {
            std::lock_guard lock{m_mutex};

            if (!std::holds_alternative<shared_observers>(m_state))
                return {};

            return std::get<shared_observers>(std::exchange(m_state, std::move(new_val)));
        }

    private:
        state_t                                                                                  m_state{};
        std::mutex                                                                               m_mutex{};
        RPP_NO_UNIQUE_ADDRESS std::conditional_t<Serialized, std::mutex, rpp::utils::none_mutex> m_serialized_mutex{};
    };
} // namespace rpp::subjects::details