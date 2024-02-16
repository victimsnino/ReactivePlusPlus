//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subjects/fwd.hpp>

#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/observers/observer.hpp>
#include <rpp/subjects/details/base_subject.hpp>
#include <rpp/subjects/details/subject_state.hpp>

#include <memory>

namespace rpp::subjects::details
{
    template<rpp::constraint::decayed_type Type>
    class serialized_strategy
    {
        struct serialized_state final : public subject_state<Type>
        {
            std::mutex mutex{};
        };

        struct observer_strategy
        {
            using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

            std::shared_ptr<serialized_state> state{};

            void set_upstream(const disposable_wrapper& d) const noexcept { state->add(d); }

            bool is_disposed() const noexcept
            {
                return state->is_disposed();
            }

            void on_next(const Type& v) const
            {
                std::lock_guard lock{state->mutex};
                state->on_next(v);
            }

            void on_error(const std::exception_ptr& err) const
            {
                std::lock_guard lock{state->mutex};
                state->on_error(err);
            }

            void on_completed() const
            {
                std::lock_guard lock{state->mutex};
                state->on_completed();
            }
        };

    public:
        using expected_disposable_strategy = rpp::details::observables::deduce_disposable_strategy_t<subject_state<Type>>;

        auto get_observer() const
        {
            return rpp::observer<Type, observer_strategy>{m_state.lock()};
        }

        template<rpp::constraint::observer_of_type<Type> TObs>
        void on_subscribe(TObs&& observer) const
        {
            m_state.lock()->on_subscribe(std::forward<TObs>(observer));
        }

        rpp::composite_disposable_wrapper get_disposable() const
        {
            return m_state;
        }

    private:
        disposable_wrapper_impl<serialized_state> m_state = disposable_wrapper_impl<serialized_state>::make();
    };
} // namespace rpp::subjects::details

namespace rpp::subjects
{
    /**
     * @brief Same as rpp::subjects::publish_subject, but on_next/on_error/on_completed calls are serialized via mutex
     *
     * @ingroup subjects
     * @see https://reactivex.io/documentation/subject.html
     */
    template<rpp::constraint::decayed_type Type>
    class serialized_subject final : public details::base_subject<Type, details::serialized_strategy<Type>>
    {
    public:
        using details::base_subject<Type, details::serialized_strategy<Type>>::base_subject;
    };
} // namespace rpp::subjects