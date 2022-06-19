#pragma once

#include <rpp/observables/details/member_overload.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/subscribers/specific_subscriber.hpp>
#include <rpp/subscriptions/composite_subscription.hpp>


namespace rpp::details
{
struct subscribe_tag;


template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, subscribe_tag>
{
    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts subscriber as is to avoid construction of dynamic_subscriber
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::subscriber_of_type<Type> TSub>
    auto subscribe(TSub&& subscriber) const
    {
        return subscribe_impl(std::forward<TSub>(subscriber));
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts observer to construct specific_subscriber without extra overheads
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::observer_of_type<Type> TObserver>
    auto subscribe(TObserver&& observer) const
    {
        return subscribe_impl<std::decay_t<TObserver>>(std::forward<TObserver>(observer));
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts subscription and observer to construct specific_subscriber without extra overheads
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::observer_of_type<Type> TObserver>
    auto subscribe(composite_subscription sub, TObserver&& observer) const
    {
        return subscribe_impl(specific_subscriber<Type, std::decay_t<TObserver>>{std::move(sub), std::forward<TObserver>(observer)});
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts raw functions to construct specific subscriber with specific observer
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::on_next_fn<Type> TOnNext      = utils::empty_function_t<Type>,
             constraint::on_error_fn      TOnError     = utils::rethrow_error_t,
             constraint::on_completed_fn  TOnCompleted = utils::empty_function_t<>>
    auto subscribe(TOnNext&& on_next = {}, TOnError&& on_error = {}, TOnCompleted&& on_completed = {}) const
    {
        return subscribe_impl(make_specific_subscriber<Type>(std::forward<TOnNext>(on_next), std::forward<TOnError>(on_error), std::forward<TOnCompleted>(on_completed)));
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts raw functions to construct specific subscriber with specific observer
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::on_next_fn<Type> TOnNext, constraint::on_completed_fn  TOnCompleted>
    auto subscribe(TOnNext&& on_next, TOnCompleted&& on_completed) const
    {
        return subscribe_impl(make_specific_subscriber<Type>(std::forward<TOnNext>(on_next), std::forward<TOnCompleted>(on_completed)));
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts subscription and raw functions to construct specific subscriber with specific observer
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::on_next_fn<Type> TOnNext      = utils::empty_function_t<Type>,
             constraint::on_error_fn      TOnError     = utils::rethrow_error_t,
             constraint::on_completed_fn  TOnCompleted = utils::empty_function_t<>>
    auto subscribe(composite_subscription sub, TOnNext&& on_next = {}, TOnError&& on_error = {}, TOnCompleted&& on_completed = {}) const
    {
        return subscribe_impl(make_specific_subscriber<Type>(std::move(sub), std::forward<TOnNext>(on_next), std::forward<TOnError>(on_error), std::forward<TOnCompleted>(on_completed)));
    }

    /**
     * \brief Main function of observable. Initiates subscription for provided subscriber and calls stored OnSubscribe function
     * \details this overloading accepts subscription raw functions to construct specific subscriber with specific observer
     * \return subscription on this observable which can be used to unsubscribe
     */
    template<constraint::on_next_fn<Type> TOnNext,
             constraint::on_completed_fn  TOnCompleted>
    auto subscribe(composite_subscription sub, TOnNext&& on_next, TOnCompleted&& on_completed) const
    {
        return subscribe_impl(make_specific_subscriber<Type>(std::move(sub), std::forward<TOnNext>(on_next), std::forward<TOnCompleted>(on_completed)));
    }

private:
    template<constraint::observer_of_type<Type> Obs>
    auto subscribe_impl(const specific_subscriber<Type, Obs>& subscriber) const
    {
        return static_cast<const SpecificObservable*>(this)->subscribe_impl(subscriber);
    }

    template<constraint::observer_of_type<Type> Obs>
    auto subscribe_impl(specific_subscriber<Type, Obs>&& subscriber) const
    {
        return static_cast<const SpecificObservable*>(this)->subscribe_impl(std::move(subscriber));
    }
};
} // namespace rpp::details
