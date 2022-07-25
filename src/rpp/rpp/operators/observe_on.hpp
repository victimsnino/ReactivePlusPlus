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

#include <rpp/operators/fwd/observe_on.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/observers/state_observer.hpp>

IMPLEMENTATION_FILE(observe_on_tag);


namespace rpp::details
{
template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct observe_on_impl
{
    [[no_unique_address]] TScheduler scheduler;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        // convert it to dynamic due to expected amount of copies == amount of items
        auto dynamic_subscriber = std::forward<TSub>(subscriber).as_dynamic();

        return create_subscriber(dynamic_subscriber);
    }

private:
    auto create_subscriber(const rpp::dynamic_subscriber<Type>& dynamic_subscriber) const
    {
        auto worker  = scheduler.create_worker(dynamic_subscriber.get_subscription());
        auto on_next = [worker](auto&& value, const rpp::dynamic_subscriber<Type>& sub)
        {
            worker.schedule([value = std::forward<decltype(value)>(value), sub]()-> schedulers::optional_duration
            {
                sub.on_next(std::move(value));
                return {};
            });
        };

        auto on_error = [worker](const std::exception_ptr& err, const rpp::dynamic_subscriber<Type>& sub)
        {
            worker.schedule([err, sub]()-> schedulers::optional_duration
            {
                sub.on_error(err);
                return {};
            });
        };

        auto on_completed = [worker](const rpp::dynamic_subscriber<Type>& sub)
        {
            worker.schedule([sub]()-> schedulers::optional_duration
            {
                sub.on_completed();
                return {};
            });
        };

        return create_subscriber_with_state<Type>(dynamic_subscriber.get_subscription().make_child(),
                                                  dynamic_subscriber,
                                                  std::move(on_next),
                                                  std::move(on_error),
                                                  std::move(on_completed));
    }
};
} // namespace rpp::details
