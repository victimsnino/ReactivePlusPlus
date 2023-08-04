//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/observable.hpp>
#include <rpp/subjects/fwd.hpp>

namespace rpp
{
template<rpp::constraint::observable OriginalObservable, rpp::constraint::subject Subject>
class connectable_observable final : public decltype(std::declval<Subject>().get_observable())
{
    using base = decltype(std::declval<Subject>().get_observable());
public:
    connectable_observable(const OriginalObservable& original_observable, const Subject& subject = Subject{})
        : base{subject.get_observable()}
        , m_original_observable{original_observable}
        , m_subject{subject} {}

    connectable_observable(OriginalObservable&& original_observable, const Subject& subject = Subject{})
        : base{subject.get_observable()}
        , m_original_observable{std::move(original_observable)}
        , m_subject{subject} {}


    rpp::disposable_wrapper connect() const
    {
        std::shared_ptr<rpp::composite_disposable> new_disposable{};

        auto current = std::atomic_load_explicit(&m_state->disposable, std::memory_order_relaxed);

        while (!m_subject.get_disposable().is_disposed())
        {
            if (current && !current->is_disposed())
                return rpp::disposable_wrapper::from_weak(current);

            if (!new_disposable)
                new_disposable = std::make_shared<rpp::composite_disposable>();

            if (!std::atomic_compare_exchange_strong(&m_state->disposable, &current, new_disposable))
                continue;

            m_original_observable.subscribe(new_disposable, m_subject.get_observer());
            return rpp::disposable_wrapper::from_weak(new_disposable);
        }

        return {};
    }

private:
    OriginalObservable                                        m_original_observable;
    Subject                                                   m_subject;
    struct state_t
    {
        rpp::utils::atomic_shared_ptr<rpp::composite_disposable> disposable{};
    };
    std::shared_ptr<state_t> m_state = std::make_shared<state_t>();
};
}