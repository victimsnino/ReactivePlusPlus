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

#include <rpp/subscribers.hpp>                       // subscribe with all types
#include <rpp/observables/interface_observable.hpp> // base_class


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

    specific_observable(const specific_observable<Type, OnSubscribeFn>&)     = default;
    specific_observable(specific_observable<Type, OnSubscribeFn>&&) noexcept = default;

    /**
     * \brief Converts rpp::specific_observable to rpp::dynamic_observable via type-erasure mechanism.
     */
    [[nodiscard]] auto as_dynamic() const & { return rpp::dynamic_observable<Type>{*this};            }
    [[nodiscard]] auto as_dynamic() &&      { return rpp::dynamic_observable<Type>{std::move(*this)}; }

    composite_subscription subscribe(const dynamic_subscriber<Type>& subscriber) const final
    {
        return subscribe_impl(subscriber);
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts subscriber as is to avoid construction of dynamic_subscriber
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::subscriber_of_type<Type> TSub>
        requires (!constraint::decayed_same_as<TSub, dynamic_subscriber<Type>>)
    composite_subscription subscribe(TSub&& subscriber) const
    {
        return subscribe_impl(subscriber);
    }

     /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts observer to construct specific_subscriber without extra overheads
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::observer_of_type<Type> TObserver>
    composite_subscription subscribe(TObserver&& observer) const
    {
        return subscribe_impl<std::decay_t<TObserver>>(std::forward<TObserver>(observer));
    }


    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts subscription and observer to construct specific_subscriber without extra overheads
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::observer_of_type<Type> TObserver>
    composite_subscription subscribe(constraint::decayed_same_as<composite_subscription> auto&& sub, TObserver&& observer) const
    {
        return subscribe_impl(specific_subscriber<Type, std::decay_t<TObserver>>{std::forward<decltype(sub)>(sub), std::forward<TObserver>(observer)});
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts raw functions to construct specific subscriber with specific observer
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<typename ...Args>
        requires (std::is_constructible_v<dynamic_subscriber<Type>, std::decay_t<Args>...> && !constraint::variadic_is_same_type<dynamic_subscriber<Type>, Args...>)
    composite_subscription subscribe(Args&&...args) const
    {
        return subscribe_impl(rpp::make_specific_subscriber<Type>(std::forward<Args>(args)...));
    }

private:
    template<constraint::observer_of_type<Type> Obs>
    composite_subscription subscribe_impl(const specific_subscriber<Type, Obs>& subscriber) const
    {
        try
        {
            m_state(subscriber);
        }
        catch (const std::runtime_error& exc)
        {
            if (subscriber.is_subscribed())
                subscriber.on_error(std::make_exception_ptr(exc));
            else
                throw;
        }
        return subscriber.get_subscription();
    }
private:
    [[no_unique_address]] OnSubscribeFn m_state;
};

template<typename OnSub>
specific_observable(OnSub on_subscribe) -> specific_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>, OnSub>;
} // namespace rpp
