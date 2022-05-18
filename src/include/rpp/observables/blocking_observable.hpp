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
class blocking_observable final
{
public:
    blocking_observable(const OriginalObservable& original)
        : m_original{original} {}

    blocking_observable(OriginalObservable&& original)
        : m_original{std::move(original)} {}


    template<constraint::subscriber_of_type<Type> TSub>
    void subscribe(TSub&& subscriber) const noexcept
    {
        subscribe_impl(subscriber);
    }

    template<constraint::observer_of_type<Type> TObserver>
    void subscribe(TObserver&& observer) const noexcept
    {
        return subscribe_impl(rpp::specific_subscriber<Type, std::decay_t<TObserver>>{std::forward<TObserver>(observer)});
    }

    template<typename ...Args>
        requires (std::is_constructible_v<dynamic_subscriber<Type>, std::decay_t<Args>...> && !constraint::variadic_is_same_type<dynamic_subscriber<Type>, Args...>)
    void subscribe(Args&&...args) const noexcept
    {
        subscribe_impl(rpp::make_specific_subscriber<Type>(std::forward<Args>(args)...));
    }
private:
    template<constraint::subscriber_of_type<Type> TSub>
    void subscribe_impl(TSub&& subscriber) const noexcept
    {
        std::promise<bool> is_success{};
        const auto         future = is_success.get_future();
        m_original.subscribe(create_subscriber_with_state<Type>(subscriber,
                                                                details::forwarding_on_next{},
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
