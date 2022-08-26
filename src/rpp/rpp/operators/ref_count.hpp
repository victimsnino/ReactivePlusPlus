//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/ref_count.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/sources/create.hpp>

IMPLEMENTATION_FILE(ref_count_tag);

namespace rpp::details
{
struct ref_count_state_t
{
    bool on_subscribe()
    {
        std::lock_guard lock{m_mutex};
        if (++m_count_of_active_subs != 1)
            return false;

        m_sub = composite_subscription{};
        return true;
    }

    void on_unsubscribe()
    {
        std::lock_guard lock{ m_mutex };
        if (--m_count_of_active_subs == 0)
            m_sub.unsubscribe();
    }

    const composite_subscription& get_subscription() const { return m_sub; }

private:
    size_t                 m_count_of_active_subs{};
    composite_subscription m_sub = composite_subscription::empty();
    std::mutex             m_mutex{};
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
struct ref_count_on_subscribe
{
    TObs observable;
    std::shared_ptr<ref_count_state_t> state = std::make_shared<ref_count_state_t>();

    template<constraint::subscriber_of_type<Type> TSub>
    void operator()(const TSub &subscriber) const
    {
        const bool need_to_connect = state->on_subscribe();

        subscriber.get_subscription().add([state = state]
                                          {
                                                state->on_unsubscribe();
                                          });

        observable.subscribe(subscriber);
        if (need_to_connect)
            observable.connect(state->get_subscription());
    }
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto ref_count_impl(TObs&& observable)
{
    return rpp::source::create<Type>(ref_count_on_subscribe<Type, std::decay_t<TObs>>{std::forward<TObs>(observable)});
}
} // namespace rpp::details
