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
#include <rpp/observables/observable.hpp>

namespace rpp::subjects::details
{
template<rpp::constraint::decayed_type T, constraint::subject_strategy<T> Strategy>
class base_subject
{
    struct on_subscribe
    {
        using ValueType = T;
        
        Strategy strategy;

        template<rpp::constraint::observer_of_type<T> TObs>
        void subscribe(TObs&& sub) const
        {
            strategy.on_subscribe(std::forward<TObs>(sub));
        }
    };
public:
    template<typename ...Args>
        requires (rpp::constraint::is_constructible_from<Strategy, Args&&...> && !rpp::constraint::variadic_decayed_same_as<base_subject, Args...>)
    explicit base_subject(Args&& ...args)
        : m_strategy{std::forward<Args>(args)...} {}

    auto get_observer() const
    {
        return m_strategy.get_observer();
    }

    auto get_observable() const
    {
        return rpp::observable<T, on_subscribe>{m_strategy};
    }

private:
    Strategy m_strategy{};
};
} // namespace rpp::subjects::details
