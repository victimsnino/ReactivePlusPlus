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

#include <rpp/disposables/refcount_disposable.hpp>
#include <rpp/observers/observer.hpp>
#include <rpp/subjects/details/base_subject.hpp>
#include <rpp/subjects/details/subject_state.hpp>

#include <memory>

namespace rpp::operators::details
{
    template<rpp::constraint::decayed_type Type>
    class forwarding_strategy
    {
        struct observer_strategy
        {
            using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

            std::shared_ptr<subjects::details::subject_state<Type>> state{};

            void set_upstream(const disposable_wrapper& d) const noexcept { state->add(d); }

            bool is_disposed() const noexcept { return state->is_disposed(); }

            void on_next(const Type& v) const { state->on_next(v); }

            void on_error(const std::exception_ptr& err) const { state->on_error(err); }

            void on_completed() const { state->on_completed(); }
        };

    public:
        using expected_disposable_strategy = typename rpp::details::observables::deduce_disposable_strategy_t<subjects::details::subject_state<Type>>::template add<1>;

        explicit forwarding_strategy(disposable_wrapper_impl<rpp::refcount_disposable> refcount)
            : m_refcount{std::move(refcount)}
        {
        }

        auto get_observer() const
        {
            return rpp::observer<Type, observer_strategy>{m_state.lock()};
        }

        template<rpp::constraint::observer_of_type<Type> TObs>
        void on_subscribe(TObs&& observer) const
        {
            if (const auto locked = m_refcount.lock())
                observer.set_upstream(locked->add_ref());
            m_state.lock()->on_subscribe(std::forward<TObs>(observer));
        }

        rpp::composite_disposable_wrapper get_disposable() const
        {
            return m_state.as_weak();
        }

    private:
        disposable_wrapper_impl<rpp::refcount_disposable>               m_refcount;
        disposable_wrapper_impl<subjects::details::subject_state<Type>> m_state = disposable_wrapper_impl<subjects::details::subject_state<Type>>::make();
    };

    template<rpp::constraint::decayed_type Type>
    using forwarding_subject = subjects::details::base_subject<Type, forwarding_strategy<Type>>;
} // namespace rpp::operators::details