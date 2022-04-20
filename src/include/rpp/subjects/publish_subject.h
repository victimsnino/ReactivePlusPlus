//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subjects/fwd.h>
#include <rpp/utils/constraints.h>
#include <rpp/sources/create.h>
#include <rpp/subscribers/dynamic_subscriber.h>
#include <rpp/subjects/subject_state.h>


namespace rpp::subjects::details
{
template<constraint::decayed_type T>
class publish_strategy
{
public:
    publish_strategy(const composite_subscription& sub = composite_subscription{})
        : m_sub{sub}
    {
        std::weak_ptr weak = m_state;
        m_sub.add([weak]
        {
            if (auto state = weak.lock())
                state->on_unsubscribe();
        });
    }

    void add(const dynamic_subscriber<T>& sub) const
    {
        m_state->on_subscribe(sub);
    }

    auto get_subscriber() const
    {
        return rpp::make_specific_subscriber<T>(m_sub,
                                                [state = m_state](const auto& v)
                                                {
                                                    state->on_next(v);
                                                },
                                                [state = m_state](const std::exception_ptr& err)
                                                {
                                                    state->on_error(err);
                                                },
                                                [state = m_state]()
                                                {
                                                    state->on_completed();
                                                });
    }

private:
    std::shared_ptr<subject_state<T>> m_state = std::make_shared<subject_state<T>>();
    composite_subscription            m_sub{};
};
} // namespace rpp::subjects::details

namespace rpp::subjects
{
template<constraint::decayed_type T>
class publish_subject
{
public:
    auto get_subscriber() const
    {
        return m_strategy.get_subscriber();
    }

    auto get_observable() const
    {
        return source::create<T>([strategy = this->m_strategy](const auto& sub)
        {
            strategy.add(sub);
        });
    }

private:
    details::publish_strategy<T> m_strategy{};
};
} // namespace rpp::subjects
