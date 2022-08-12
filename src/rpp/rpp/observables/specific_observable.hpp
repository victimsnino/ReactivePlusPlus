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

#include <rpp/defs.hpp>
#include <rpp/observables/fwd.hpp>
#include <rpp/observables/interface_observable.hpp>            // base_class
#include <rpp/schedulers/trampoline_scheduler.hpp>
#include <rpp/subscribers/dynamic_subscriber.hpp>
#include <rpp/utils/operator_declaration.hpp>                  // for header include
#include <rpp/utils/utilities.hpp>                            // copy_assignable_callable

#include <utility>

namespace rpp
{
/**
 * \brief observable specified with specific type of OnSubscribeFn. Used to store OnSubscribeFn function as is on stack (instead of allocating it somewhere).
 *
 * It has better performance comparing to rpp::dynamic_observable. Use it if possible. But it has worse usability due to OnSubscribeFn template parameter.
 * \tparam Type is type of value provided by this observable
 * \tparam OnSubscribeFn is type of function/functor/callable used during subscription on this observable
 * \ingroup observables
 */
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
class specific_observable : public interface_observable<Type, specific_observable<Type, OnSubscribeFn>>
{
public:
    specific_observable(OnSubscribeFn&& on_subscribe)
        : m_state{ std::move(on_subscribe) } {}

    specific_observable(const OnSubscribeFn& on_subscribe)
        : m_state{ on_subscribe } {}

    specific_observable(const specific_observable& other)                    = default;
    specific_observable(specific_observable&& other) noexcept                = default;
    specific_observable& operator=(const specific_observable& other)         = default;
    specific_observable& operator=(specific_observable&& other) noexcept     = default;
    /**
     * \brief Converts rpp::specific_observable to rpp::dynamic_observable via type-erasure mechanism.
     */
    template <typename...Args>
    [[nodiscard]] auto as_dynamic() const & requires details::is_header_included<details::dynamic_observable_tag, Args...> { return rpp::dynamic_observable<Type>{*this};            }
    template <typename...Args>
    [[nodiscard]] auto as_dynamic() && requires details::is_header_included<details::dynamic_observable_tag, Args...>    { return rpp::dynamic_observable<Type>{std::move(*this)}; }

    friend struct details::member_overload<Type, specific_observable<Type, OnSubscribeFn>, details::subscribe_tag>;

private:

    // used by rpp::details::member_overload<Type, specific_observable<Type, OnSubscribeFn>, rpp::details::subscribe_tag>;
    template<constraint::subscriber_of_type<Type> TSub>
    composite_subscription subscribe_impl(const TSub& subscriber) const
    {
        if (subscriber.is_subscribed())
            actual_subscribe(subscriber);

        return subscriber.get_subscription();
    }

    template<constraint::subscriber_of_type<Type> TSub>
    void actual_subscribe(const TSub& subscriber) const
    {
        // will be scheduled immediately -> reference can be passed
        const auto safe_subscribe = [&]
        {
            try
            {
                m_state(subscriber);
            }
            catch (...)
            {
                if (subscriber.is_subscribed())
                    subscriber.on_error(std::current_exception());
                else
                    throw;
            }
            return rpp::schedulers::optional_duration{};
        };

        // take ownership over current thread as early as possible to delay all next "current_thread" schedulings. For  example, scheduling of emissions from "just" to delay it till whole chain is subscribed and ready to listened emissions
        // For example, if we have
        // rpp::source::just(rpp::schedulers::current_thread{}, 1,2).combine_latest(rpp::source::just(rpp::schedulers::current_thread{}, 1,2))
        //
        // then we expect to see emissions like (1,1) (2,1) (2,2) instead of (2,1) (2,2). TO do it we need to "take ownership" over queue to prevent ANY immediate schedulings from ANY next subscriptions
        if (rpp::schedulers::current_thread::is_queue_owned())
        {
            safe_subscribe();
        }
        else
        {
            rpp::schedulers::current_thread::create_worker(subscriber.get_subscription()).schedule(safe_subscribe);
        }

    }

private:
    RPP_NO_UNIQUE_ADDRESS OnSubscribeFn m_state;
};

template<typename OnSub>
specific_observable(OnSub on_subscribe) -> specific_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>, OnSub>;
} // namespace rpp
