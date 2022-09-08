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
 * \brief Type-full observable (or typed) that has the notion of Type and upstream observables for C++ compiler. e.g. observable<int, map<bool, ...recursive...>> is different from observable<int, filter<int, ...>>.
 *
 * \details This is a C++ technique about de-virtualization. To achieve polymorphic behavior, we could either go for function virtualization or function overload. 
 * However, virtualization is more expensive than function overload in both compile time and runtime. 
 * Therefore, we go for function overload. Actually, we use more advanced functor paradigm for better performance.
 * As a result it has better performance comparing to rpp::dynamic_observable. Use it if possible. But it has worse usability due to OnSubscribeFn template parameter.
 *
 * \param Type is the value type. Observable of type means this source could emit a sequence of items of that "Type".
 * \param OnSubscribeFn is the on_subscribe functor that is called when a subscriber subscribes to this observable. specific_observable stores OnSubscribeFn as member variable, so, it is stored on stack (instead of allocating it on heap).
 * \ingroup observables
 */
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
class specific_observable : public interface_observable<Type, specific_observable<Type, OnSubscribeFn>>
{
public:
    specific_observable(OnSubscribeFn&& on_subscribe)
        : m_on_subscribe{std::move(on_subscribe) } {}

    specific_observable(const OnSubscribeFn& on_subscribe)
        : m_on_subscribe{on_subscribe } {}

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
                m_on_subscribe(subscriber);
            }
            catch (...)
            {
                if (subscriber.is_subscribed())
                    subscriber.on_error(std::current_exception());
                else
                    throw;
            }
            return schedulers::optional_duration{};
        };

        // take ownership over current thread as early as possible to delay all next "current_thread" schedulings. For  example, scheduling of emissions from "just" to delay it till whole chain is subscribed and ready to listened emissions
        // For example, if we have
        // rpp::source::just(rpp::schedulers::current_thread{}, 1,2).combine_latest(rpp::source::just(rpp::schedulers::current_thread{}, 1,2))
        //
        // then we expect to see emissions like (1,1) (2,1) (2,2) instead of (2,1) (2,2). TO do it we need to "take ownership" over queue to prevent ANY immediate schedulings from ANY next subscriptions
        if (schedulers::current_thread::is_queue_owned())
        {
            safe_subscribe();
        }
        else
        {
            // we need to submit work into queue to take ownership over it. We can submit work with time_point "zero" due to anyway queue is empty and it doesn't make sense, but we can take performance boost due to avoiding extra calls to "now"
            schedulers::current_thread::create_worker(subscriber.get_subscription()).schedule(schedulers::time_point{}, safe_subscribe);
        }

    }

private:
    /**
     * \brief The on_subscribe functor, which has the operator()(const auto& subscriber) overload function.
     */
    RPP_NO_UNIQUE_ADDRESS OnSubscribeFn m_on_subscribe;
};

#if defined(RPP_TYPE_ERASED_OBSERVABLE) && RPP_TYPE_ERASED_OBSERVABLE
template<typename OnSub>
specific_observable(OnSub on_subscribe) -> specific_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>>;
#else
template<typename OnSub>
specific_observable(OnSub on_subscribe) -> specific_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>, OnSub>;
#endif
} // namespace rpp
