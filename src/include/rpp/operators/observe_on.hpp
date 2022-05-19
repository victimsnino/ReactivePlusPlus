#pragma once

#include <rpp/operators/fwd/observe_on.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/observers/state_observer.hpp>

IMPLEMENTATION_FILE(observe_on_tag);


namespace rpp::details
{
template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
auto observe_on_impl(TScheduler&& scheduler)
{
    return[scheduler = std::forward<TScheduler>(scheduler)]<constraint::subscriber_of_type<Type> TSub>(TSub && subscriber)
    {
        // convert it to dynamic due to expected amount of copies == amount of items
        auto dynamic_subscriber = std::forward<TSub>(subscriber).as_dynamic();
        auto subscription = dynamic_subscriber.get_subscription();
        auto worker = scheduler.create_worker(subscription);

        auto on_next = [worker](auto&& value, const constraint::subscriber_of_type<Type> auto& sub)
        {
            worker.schedule([value = std::forward<decltype(value)>(value), sub]()->schedulers::optional_duration
            {
                sub.on_next(std::move(value));
                return {};
            });
        };

        auto on_completed = [worker](const constraint::subscriber_of_type<Type> auto& sub)
        {
            worker.schedule([sub]()->schedulers::optional_duration
            {
                sub.on_completed();
                return {};
            });
        };

        return create_subscriber_with_state<Type>(subscription.make_child(), std::move(dynamic_subscriber), std::move(on_next), forwarding_on_error{}, std::move(on_completed));
    };
}
} // namespace rpp::details
