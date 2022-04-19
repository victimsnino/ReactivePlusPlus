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

namespace rpp::subjects::details
{
template<constraint::decayed_type T>
class publish_strategy
{
    using subscribers = std::vector<dynamic_subscriber<T>>;
public:
    void add(const dynamic_subscriber<T>& sub) const
    {
        if (!sub.is_subscribed())
            return;

        while (true)
        {
            auto current_subs = m_state->subs.load();
            auto new_subs     = copy_subscribed_subs(current_subs, current_subs->size() + 1);
            new_subs->emplace_back(sub);

            if (!m_state->subs.compare_exchange_strong(current_subs, new_subs))
                continue;

            std::weak_ptr weak = m_state;
            sub.get_subscription().add([weak]
            {
                auto state = weak.lock();
                if (!state)
                    return;

                while (true)
                {
                    auto current_subs = state->subs.load();
                    auto new_subs = copy_subscribed_subs(current_subs, std::max(current_subs->size(), size_t{ 1 }) - 1);

                    if (!state->subs.compare_exchange_strong(current_subs, new_subs))
                        break;
                }
            });
            return;
        }
    }

    auto get_subscriber() const
    {
        return rpp::make_specific_subscriber<T>(m_sub,
            [state = m_state](const auto& v)
            {
                auto observers = state->subs.load();
                std::ranges::for_each(*observers,
                    [&v](const auto& obs)
                    {
                        obs.on_next(v);
                    });
            },
            [state = m_state](const std::exception_ptr& err)
            {
                auto observers = state->subs.load();
                std::ranges::for_each(*observers,
                    [&err](const auto& obs)
                    {
                        obs.on_error(err);
                    });
            },
            [state = m_state]()
            {
                auto observers = state->subs.load();
                std::ranges::for_each(*observers,
                    [](const auto& obs)
                    {
                        obs.on_completed();
                    });
            });
    }

private:
    static auto copy_subscribed_subs(std::shared_ptr<subscribers> current_subs, size_t expected_size)
    {
        auto new_subs = std::make_shared<subscribers>();
        new_subs->reserve(expected_size);
        std::ranges::copy_if(*current_subs, std::back_inserter(*new_subs), std::mem_fn(&dynamic_subscriber<T>::is_subscribed));
        return new_subs;
    }
private:
    struct state_t
    {
        std::atomic<std::shared_ptr<subscribers>> subs = std::make_shared<subscribers>();
    };

    composite_subscription   m_sub{};
    std::shared_ptr<state_t> m_state = std::make_shared<state_t>();
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
