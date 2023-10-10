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

#include <rpp/observers/observer.hpp>
#include <rpp/subjects/details/base_subject.hpp>
#include <rpp/subjects/details/subject_state.hpp>
#include <rpp/disposables/refcount_disposable.hpp>

#include <memory>

namespace rpp::operators::details
{
template<rpp::constraint::decayed_type Type>
class forwarding_strategy
{
    struct observer_strategy
    {
        std::shared_ptr<subjects::details::subject_state<Type>> state{};

        void set_upstream(const disposable_wrapper& d) const noexcept { state->add(d); }

        bool is_disposed() const noexcept { return state->is_disposed(); }

        void on_next(const Type& v) const { state->on_next(v); }

        void on_error(const std::exception_ptr& err) const { state->on_error(err); }

        void on_completed() const { state->on_completed(); }
    };

public:
    explicit forwarding_strategy(std::shared_ptr<rpp::refcount_disposable> refcount)
        : m_refcount{std::move(refcount)}
    {
        m_refcount->add(rpp::disposable_wrapper::from_weak(m_state));
    }

    auto get_observer() const
    {
        return rpp::observer<Type, rpp::details::with_external_disposable<observer_strategy>>{composite_disposable_wrapper{m_state}, observer_strategy{m_state}};
    }

    template<rpp::constraint::observer_of_type<Type> TObs>
    void on_subscribe(TObs&& observer) const
    {
        observer.set_upstream(m_refcount->add_ref());
        m_state->on_subscribe(std::forward<TObs>(observer));
    }

    rpp::disposable_wrapper get_disposable() const
    {
        return rpp::disposable_wrapper{m_state};
    }

private:
    std::shared_ptr<subjects::details::subject_state<Type>> m_state = std::make_shared<subjects::details::subject_state<Type>>();
    std::shared_ptr<rpp::refcount_disposable>               m_refcount{};
};

template<rpp::constraint::decayed_type Type>
using forwarding_subject = subjects::details::base_subject<Type, forwarding_strategy<Type>>;
} 