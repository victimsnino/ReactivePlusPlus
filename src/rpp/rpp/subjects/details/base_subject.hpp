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
#include <rpp/sources/create.hpp>

namespace rpp::subjects::details
{
template<rpp::constraint::decayed_type T, constraint::subject_strategy<T> Strategy>
class base_subject
{
public:
    template<typename ...Args>
        requires (rpp::constraint::is_constructible_from<Strategy, Args&&...> && !rpp::constraint::variadic_decayed_same_as<base_subject, Args...>)
    explicit base_subject(Args&& ...args)
        : m_strategy{std::forward<Args>(args)...} {}

    auto get_subscriber() const
    {
        return m_strategy.get_subscriber();
    }

    auto get_observable() const
    {
        return rpp::source::create<T>([strategy = this->m_strategy](auto&& sub)
        {
            strategy.on_subscribe(std::forward<decltype(sub)>(sub));
        });
    }

private:
    Strategy m_strategy{};
};
} // namespace rpp::subjects::details
