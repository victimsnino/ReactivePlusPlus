//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subjects/details/base_subject.hpp>
#include <rpp/subjects/details/subject_state.hpp>
#include <rpp/subjects/fwd.hpp>
#include <rpp/observers/lambda_observer.hpp>

#include <memory>

namespace rpp::subjects::details
{
template<rpp::constraint::decayed_type Type>
class publish_strategy
{
public:
    auto get_observer() const
    {
        return rpp::make_lambda_observer<Type>([state = m_state](const Type& v) { state->on_next(v); },
                                               [state = m_state](const std::exception_ptr& err) { state->on_error(err); },
                                               [state = m_state]() { state->on_completed(); });
    }

    template<rpp::constraint::observer_of_type<Type> TObs>
    void on_subscribe(TObs&& observer) const
    {
        m_state->on_subscribe(std::forward<TObs>(observer));
    }

private:
    std::shared_ptr<subject_state<Type>> m_state = std::make_shared<subject_state<Type>>();
};
} // namespace rpp::subjects::details