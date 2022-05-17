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
class publish_strategy
{
public:
    publish_strategy(const composite_subscription& sub)
        : m_sub{sub}
    {
        m_sub.add([state = m_state]
        {
            state->on_unsubscribe();
        });
    }

    void on_subscribe(const dynamic_subscriber<T>& sub) const
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
/**
 * \brief Subject which just multicasts values to observers subscribed on it. It contains two parts: subscriber and observable at the same time.
 * \details Each subscriber obtains only values which emitter after corresponding subscribe. on_error/on_completer/unsubscribe cached and provided to new subscribers if any
 * \warn this subject is not synchronized/serialized! It means, that expected to call callbacks of subscriber in the serialized way to follow observable contract: "Observables must issue notifications to observers serially (not in parallel).". If you are not sure or need extra serialization, please, use serialized_subject.
 * \tparam T value provided by this subject
 * \see https://reactivex.io/documentation/subject.html
 * \ingroup subjects
 */
template<rpp::constraint::decayed_type T>
class publish_subject final : public details::base_subject<T, details::publish_strategy<T>>{};
} // namespace rpp::subjects
