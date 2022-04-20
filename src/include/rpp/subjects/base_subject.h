//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/utils/constraints.h>
#include <rpp/sources/create.h>
#include <rpp/subscribers/constraints.h>
#include <rpp/subscribers/fwd.h>
#include <rpp/subscriptions/composite_subscription.h>

namespace rpp::subjects::details
{
template<typename Strategy, typename T>
concept subject_strategy = std::constructible_from<Strategy, rpp::composite_subscription> && requires(Strategy t)
{
    {t.get_subscriber()} -> rpp::constraint::subscriber;
    t.on_subscribe(std::declval<rpp::dynamic_subscriber<T>>());
};

template<constraint::decayed_type T, subject_strategy<T> Strategy>
class base_subject
{
public:
    base_subject(const composite_subscription& sub = composite_subscription{})
        : m_strategy{sub} {}

    auto get_subscriber() const
    {
        return m_strategy.get_subscriber();
    }

    auto get_observable() const
    {
        return source::create<T>([strategy = this->m_strategy](const auto& sub)
        {
            strategy.on_subscribe(sub);
        });
    }

private:
    Strategy m_strategy{};
};
} // namespace rpp::subjects::details