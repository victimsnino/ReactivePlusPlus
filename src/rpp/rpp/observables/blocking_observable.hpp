//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/constraints.hpp>         // OriginalObservable type
#include <rpp/subscribers/specific_subscriber.hpp> // create subscriber
#include <rpp/observers/state_observer.hpp>        // wrap subscribers

#include <rpp/observables/details/member_overload.hpp> // overload operators
#include <rpp/operators/fwd.hpp>                    // forwarding of member_overaloads
#include <rpp/defs.hpp>                             // RPP_EMPTY_BASES

#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state

#include <future>

namespace rpp
{
/**
 * \brief blocking alternative of observable: provides interface where each function do blocking subscribe on original observable (waits till on_completed and provides value)
 * \tparam Type type of values emitted by this observable
 * \tparam OriginalObservable original observable wrapped by this observable
 * \ingroup observables
 */
template<constraint::decayed_type Type, constraint::observable_of_type<Type> OriginalObservable>
class RPP_EMPTY_BASES blocking_observable final : public details::member_overload<Type, blocking_observable<Type, OriginalObservable>, details::subscribe_tag>
{
public:
    blocking_observable(const OriginalObservable& original)
        : m_original{original} {}

    blocking_observable(OriginalObservable&& original)
        : m_original{std::move(original)} {}

    friend struct details::member_overload<Type, blocking_observable<Type, OriginalObservable>, details::subscribe_tag>;

private:
    template<constraint::subscriber_of_type<Type> TSub>
    void subscribe_impl(TSub&& subscriber) const noexcept
    {
        std::promise<bool> is_success{};
        const auto         future = is_success.get_future();
        m_original.subscribe(create_subscriber_with_state<Type>(std::forward<TSub>(subscriber),
                                                                utils::forwarding_on_next{},
                                                                [&](const std::exception_ptr& err, const auto& sub)
                                                                {
                                                                    sub.on_error(err);
                                                                    is_success.set_value(false);
                                                                },
                                                                [&](const auto& sub)
                                                                {
                                                                    sub.on_completed();
                                                                    is_success.set_value(true);
                                                                }));
        future.wait();
    }

private:
    OriginalObservable m_original;
};

template<constraint::observable TObs>
blocking_observable(const TObs&)->blocking_observable<rpp::utils::extract_observable_type_t<TObs>, TObs>;
} // namespace rpp
