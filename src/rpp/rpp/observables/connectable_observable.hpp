//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/constraints.hpp>              // OriginalObservable type
#include <rpp/operators/fwd/ref_count.hpp>              // include forwarding for member_overload
#include <rpp/subjects/constraints.hpp>                 // type of subject used
#include <rpp/subjects/type_traits.hpp>                 // deduce observable type by subject type
#include <rpp/subscriptions/composite_subscription.hpp> // lifetime
#include <rpp/defs.hpp>                                 // RPP_EMPTY_BASES

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state


#include <memory>
#include <mutex>

namespace rpp
{
/**
 * \brief connectable alternative of observable: extends interface with extra functionality.
 *  Common subscription will subscribe on underlying subject, but connect/ref_count will initiate subscription on original observable
 * \tparam Type type of values emitted by this observable
 * \tparam OriginalObservable original observable wrapped by this observable
 * \see https://reactivex.io/documentation/operators/publish.html
 * \ingroup observables
 */
template<constraint::decayed_type                    Type,
         subjects::constraint::subject_of_type<Type> Subject,
         constraint::observable_of_type<Type>        OriginalObservable>
class RPP_EMPTY_BASES connectable_observable
    : public decltype(std::declval<Subject>().get_observable())
    , public details::member_overload<Type, connectable_observable<Type, Subject, OriginalObservable>, details::ref_count_tag>
{
    using base = decltype(std::declval<Subject>().get_observable());
public:
    connectable_observable(const OriginalObservable& original_observable, const Subject& subject = Subject{})
        : base{subject.get_observable()}
        , m_original_observable{original_observable}
        , m_state{std::make_shared<state_t>(subject)} {}

    connectable_observable(OriginalObservable&& original_observable, const Subject& subject = Subject{})
        : base{subject.get_observable()}
        , m_original_observable{std::move(original_observable)}
        , m_state{std::make_shared<state_t>(subject)} {}

    composite_subscription connect(const composite_subscription& subscription = composite_subscription{}) const
    {
        auto        subscriber              = m_state->subject.get_subscriber();
        const auto& subscriber_subscription = subscriber.get_subscription();
        
        {
            std::lock_guard lock(m_state->mutex);

            if (!m_state->sub.is_empty())
                return subscription;

            subscriber_subscription.add(subscription);
            m_state->sub = subscription;
        }

        subscription.add([state = std::weak_ptr{m_state}]
        {
            if (const auto locked = state.lock())
            {
                auto current_sub = composite_subscription::empty();
                {
                    std::lock_guard lock(locked->mutex);
                    std::swap(current_sub, locked->sub);
                }
                current_sub.unsubscribe();
                locked->subject.get_subscriber().get_subscription().remove(current_sub);
            }
        });


        m_original_observable.subscribe(create_subscriber_with_state<Type>(m_state->sub,
                                                                                  utils::forwarding_on_next{},
                                                                                  utils::forwarding_on_error{},
                                                                                  utils::forwarding_on_completed{},
                                                                                  subscriber.get_observer(),
                                                                                  // capture state to be sure that state is alive while ANY subscriber is alive
                                                                                  m_state));

        return subscription;
    }

private:
    OriginalObservable     m_original_observable;
    struct state_t
    {
        state_t(const Subject& subj) : subject{subj} {}
        
        Subject                subject;
        std::mutex             mutex{};
        composite_subscription sub = composite_subscription::empty();
    };

    std::shared_ptr<state_t> m_state{};
};

template<constraint::observable OriginalObservable, subjects::constraint::subject Subject>
connectable_observable(const OriginalObservable&, const Subject&) -> connectable_observable<subjects::utils::extract_subject_type_t<Subject>, Subject, OriginalObservable>;
} // namespace rpp
