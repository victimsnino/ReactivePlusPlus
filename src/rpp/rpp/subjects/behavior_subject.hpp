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
#include <rpp/utils/constraints.hpp>
#include <rpp/subscribers/dynamic_subscriber.hpp>
#include <rpp/subjects/details/subject_state.hpp>
#include <rpp/subjects/details/base_subject.hpp>

namespace rpp::subjects::details
{
template<rpp::constraint::decayed_type T>
class behavior_strategy
{
public:
    template<rpp::constraint::decayed_same_as<T> TT>
    behavior_strategy(TT&& v, const composite_subscription& sub)
        : m_state{std::make_shared<behavior_state>(std::forward<TT>(v))}
        , m_sub{sub}
    {
        m_sub.add([state = std::weak_ptr{m_state}]
        {
            if (const auto locked = state.lock())
                locked->on_unsubscribe();
        });
    }

    void on_subscribe(const dynamic_subscriber<T>& sub) const
    {
        if (m_sub.is_subscribed())
            sub.on_next(m_state->get_value());

        m_state->on_subscribe(sub);
    }

    auto get_subscriber() const
    {
        return rpp::make_specific_subscriber<T>(m_sub,
                                                [state = m_state](const T& v)
                                                {
                                                    state->set_value(v);
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

    T get_value() const
    {
        return m_state->get_value();
    }

private:
    class behavior_state : public subject_state<T>
    {
    public:
        behavior_state(const T& v)
            : subject_state<T>{}
            , value{v} {}

        behavior_state(T&& v)
            : subject_state<T>{}
            , value{std::move(v)} {}

        T get_value()
        {
            std::lock_guard lock{mutex};
            return value;
        }


        void set_value(const T& v)
        {
            std::lock_guard lock{mutex};
            value = v;
        }

    private:

        std::mutex mutex;
        T          value;
    };

    std::shared_ptr<behavior_state> m_state;
    composite_subscription          m_sub{};
};
} // namespace rpp::subjects::details

namespace rpp::subjects
{
/**
 * \brief Subject which multicasts values to observers subscribed on it and sends last emitted value (or initial value) on subscribe. It contains two parts: subscriber and observable at the same time.
 * 
 * \details Each subscriber obtains only last/initial value + values which emitted after corresponding subscribe. on_error/on_completer/unsubscribe cached and provided to new subscribers if any
 * 
 * \warning this subject is not synchronized/serialized! It means, that expected to call callbacks of subscriber in the serialized way to follow observable contract: "Observables must issue notifications to observers serially (not in parallel).". If you are not sure or need extra serialization, please, use serialized_subject.
 * 
 * \tparam T value provided by this subject
 * 
 * \ingroup subjects
 * \see https://reactivex.io/documentation/subject.html
 */
template<rpp::constraint::decayed_type T>
class behavior_subject final : public details::base_subject<T, details::behavior_strategy<T>>
{
public:
    behavior_subject(const T& initial_value, const composite_subscription& sub = composite_subscription{})
        : details::base_subject<T, details::behavior_strategy<T>>{initial_value, sub} {}

    behavior_subject(T&& initial_value, const composite_subscription& sub = composite_subscription{})
        : details::base_subject<T, details::behavior_strategy<T>>{std::move(initial_value), sub} {}

    T get_value() const
    {
        return details::base_subject<T, details::behavior_strategy<T>>::get_strategy().get_value();
    }
};
} // namespace rpp::subjects
