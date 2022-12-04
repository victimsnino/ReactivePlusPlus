//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/utils/constraints.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/subscriptions/composite_subscription.hpp>
#include <rpp/subjects/fwd.hpp>

namespace rpp::subjects::details
{
struct subject_tag{};

template<rpp::constraint::decayed_type T, subject_strategy<T> Strategy>
class base_subject : public subject_tag
{
public:
    base_subject(const composite_subscription& sub = composite_subscription{})
        : m_strategy{sub} {}

    base_subject(composite_subscription&& sub)
        : m_strategy{std::move(sub)} {}

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

protected:
    base_subject(auto&& ...args)
        : m_strategy{std::forward<decltype(args)>(args)...} {}

    const Strategy& get_strategy() const { return m_strategy; }

private:
    Strategy m_strategy{};
};
} // namespace rpp::subjects::details
