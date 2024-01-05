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
#include <rpp/subjects/details/subject_state.hpp>

#include <memory>

namespace rpp::subjects::details
{
template<rpp::constraint::decayed_type Type>
class serialized_strategy
{
    struct serialized_state
    {
        std::mutex          mutex{};
        subject_state<Type> state{};
    };

    struct observer_strategy
    {
        std::shared_ptr<serialized_state> state{};

        void set_upstream(const disposable_wrapper& d) const noexcept { state->state.add(d); }

        bool is_disposed() const noexcept 
        { 
            return state->state.is_disposed(); 
        }

        void on_next(const Type& v) const 
        {
            std::lock_guard lock{state->mutex};
            state->state.on_next(v); 
        }

        void on_error(const std::exception_ptr& err) const 
        {
            std::lock_guard lock{state->mutex};
            state->state.on_error(err); 
        }

        void on_completed() const 
        {
            std::lock_guard lock{state->mutex};
            state->state.on_completed(); 
        }
    };

public:

    using expected_disposable_strategy = rpp::details::observables::deduce_disposable_strategy_t<subject_state<Type>>;

    auto get_observer() const
    {
        return rpp::observer<Type, rpp::details::with_external_disposable<observer_strategy>>{composite_disposable_wrapper{m_state->state}, observer_strategy{m_state}};
    }

    template<rpp::constraint::observer_of_type<Type> TObs>
    void on_subscribe(TObs&& observer) const
    {
        m_state->state.on_subscribe(std::forward<TObs>(observer));
    }

    rpp::disposable_wrapper get_disposable() const
    {
        return rpp::disposable_wrapper{m_state->state};
    }

private:
    std::shared_ptr<serialized_state> m_state = std::make_shared<serialized_state>();
};
} // namespace rpp::subjects::details