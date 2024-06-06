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
#include <deque>
#include <memory>
#include <mutex>
#include <variant>

namespace rpp::subjects::details
{
    struct completed
    {
    };

    struct disposed
    {
    };

    template<rpp::constraint::decayed_type Type, bool Serialized>
    class subject_state : public composite_disposable
        , public rpp::details::enable_wrapper_from_this<subject_state<Type, Serialized>>
    {
        template<rpp::constraint::observer TObs>
        class disposable_with_observer : public rpp::details::observers::type_erased_observer<TObs>
            , public rpp::details::base_disposable
        {
        public:
            disposable_with_observer(TObs&& observer, std::weak_ptr<subject_state> state)
                : rpp::details::observers::type_erased_observer<TObs>{std::move(observer)}
                , m_state{std::move(state)}
            {
            }

        private:
            void base_dispose_impl(interface_disposable::Mode) noexcept override
            {
                if (const auto shared = m_state.lock())
                {
                    std::unique_lock lock{shared->m_mutex};
                    process_state_unsafe(shared->m_state,
                                         [&](const shared_observers& observers) {
                                             shared->m_state = cleanup_observers(observers, this);
                                         });
                }
            }

            std::weak_ptr<subject_state> m_state{};
        };

        using observer         = std::shared_ptr<rpp::details::observers::observer_vtable<Type>>;
        using observers        = std::deque<observer>;
        using shared_observers = std::shared_ptr<observers>;
        using state_t          = std::variant<shared_observers, std::exception_ptr, completed, disposed>;

    public:
        using expected_disposable_strategy = rpp::details::observables::atomic_fixed_disposable_strategy_selector<1>;

        subject_state() = default;

        template<rpp::constraint::observer_of_type<Type> TObs>
        void on_subscribe(TObs&& observer)
        {
            std::unique_lock lock{m_mutex};
            process_state_unsafe(
                m_state,
                [&](shared_observers& observers) {
                    auto d   = disposable_wrapper_impl<disposable_with_observer<std::decay_t<TObs>>>::make(std::forward<TObs>(observer), this->wrapper_from_this().lock());
                    auto ptr = d.lock();

                    if (!observers)
                        observers = std::make_shared<subject_state::observers>();
                    observers->emplace_back(ptr);

                    lock.unlock();
                    ptr->set_upstream(d.as_weak());
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
            std::unique_lock observers_lock{m_mutex};

            if (!std::holds_alternative<shared_observers>(m_state))
                return;

            // we are getting copy of curent deque and obtaining CURRENT begin/end of in case of some new observer would be added during on_next call
            const auto observers = std::get<shared_observers>(m_state);
            if (!observers)
                return;

            const auto begin = observers->cbegin();
            const auto end   = observers->cend();

            observers_lock.unlock();

            std::lock_guard lock{m_serialized_mutex};
            std::for_each(begin, end, [&](const observer& obs) { obs->on_next(v); });
        }

        void on_error(const std::exception_ptr& err)
        {
            {
                std::lock_guard lock{m_serialized_mutex};
                if (const auto observers = exchange_observers_under_lock_if_there(err))
                    rpp::utils::for_each(*observers, [&](const observer& obs) { obs->on_error(err); });
            }
            dispose();
        }

        void on_completed()
        {
            {
                std::lock_guard lock{m_serialized_mutex};
                if (const auto observers = exchange_observers_under_lock_if_there(completed{}))
                    rpp::utils::for_each(*observers, [](const observer& obs) { obs->on_completed(); });
            }
            dispose();
        }

    private:
        void composite_dispose_impl(interface_disposable::Mode) noexcept override
        {
            exchange_observers_under_lock_if_there(disposed{});
        }

        static shared_observers cleanup_observers(const shared_observers& current_subs, const rpp::details::observers::observer_vtable<Type>* to_delete)
        {
            auto subs = std::make_shared<observers>();
            if (current_subs)
            {
                std::copy_if(current_subs->cbegin(),
                             current_subs->cend(),
                             std::back_inserter(*subs),
                             [&to_delete](const observer& obs) {
                                 return to_delete != obs.get() && !obs->is_disposed();
                             });
            }
            return subs;
        }

        static void process_state_unsafe(const state_t& state, const auto&... actions)
        {
            std::visit(rpp::utils::overloaded{actions..., rpp::utils::empty_function_any_t{}}, state);
        }

        shared_observers exchange_observers_under_lock_if_there(state_t&& new_val)
        {
            std::lock_guard lock{m_mutex};

            if (!std::holds_alternative<shared_observers>(m_state))
                return {};

            return std::get<shared_observers>(std::exchange(m_state, std::move(new_val)));
        }

    private:
        state_t                                                                                  m_state;
        std::mutex                                                                               m_mutex{};
        RPP_NO_UNIQUE_ADDRESS std::conditional_t<Serialized, std::mutex, rpp::utils::none_mutex> m_serialized_mutex{};
    };
} // namespace rpp::subjects::details
