//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                    TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/defs.hpp>                                    // RPP_NO_UNIQUE_ADDRESS
#include <rpp/operators/lift.hpp>                          // required due to operator uses lift
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/on_error_resume_next.hpp>      // own forwarduing
#include <rpp/subscribers/constraints.hpp>                 // constraint::subscriber_of_type
#include <rpp/utils/functors.hpp>

IMPLEMENTATION_FILE(on_error_resume_next_tag);

namespace rpp::details
{
/**
 * Functor (type-erasure) of "on_error_resume_next" for on_error operator.
 */
struct on_error_resume_next_on_error
{
    template<rpp::details::resume_callable ResumeCallable>
    void operator()(const std::exception_ptr& err,
                    const auto& subscriber,
                    const ResumeCallable& resume_callable) const
    {
        using Type = rpp::utils::extract_subscriber_type_t<decltype(subscriber)>;

        // Subscribe to next_observable
        resume_callable(err).subscribe(subscriber);
    }
};

/**
 * \brief Functor of OperatorFn for "on_error_resume_next" operator (used by "lift").
 */
template<constraint::decayed_type Type, rpp::details::resume_callable ResumeCallable>
struct on_error_resume_next_impl
{
    RPP_NO_UNIQUE_ADDRESS ResumeCallable resume_callable;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& downstream_subscriber) const
    {
        // Child subscription is for keeping the downstream subscriber's subscription alive when upstream sends on_error event.
        auto subscription = downstream_subscriber.get_subscription().make_child();

        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  rpp::utils::forwarding_on_next{},
                                                  on_error_resume_next_on_error{},
                                                  rpp::utils::forwarding_on_completed{},
                                                  std::forward<TSub>(downstream_subscriber),
                                                  resume_callable);
    }
};
} // namespace rpp::details
