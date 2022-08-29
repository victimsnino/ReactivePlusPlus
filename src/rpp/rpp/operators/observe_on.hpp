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
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/observe_on.hpp>
#include <rpp/subscribers/constraints.hpp>


IMPLEMENTATION_FILE(observe_on_tag);


namespace rpp::details
{
struct observe_on_on_next
{
    void operator()(auto&& value, const auto& sub, const auto& worker) const
    {
        worker.schedule([value = std::forward<decltype(value)>(value), sub]
                        {
                            sub.on_next(std::move(value));
                            return schedulers::optional_duration{};
                        });
    }
};

struct observe_on_on_error
{
    void operator()(const std::exception_ptr& err, const auto& sub, const auto& worker) const
    {
        worker.schedule([err, sub]
                        {
                            sub.on_error(err);
                            return schedulers::optional_duration{};
                        });
    }
};

struct observe_on_on_completed
{
    void operator()(const auto& sub, const auto& worker) const
    {
        worker.schedule([sub]
                        {
                            sub.on_completed();
                            return schedulers::optional_duration{};
                        });
    }
};

template<constraint::decayed_type Type, schedulers::constraint::scheduler TScheduler>
struct observe_on_impl
{
    RPP_NO_UNIQUE_ADDRESS TScheduler scheduler;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        // convert it to dynamic due to expected amount of copies == amount of items
        auto dynamic_subscriber = std::forward<TSub>(subscriber).as_dynamic();

        auto worker = scheduler.create_worker(dynamic_subscriber.get_subscription());

        return create_subscriber_with_state<Type>(dynamic_subscriber.get_subscription().make_child(),
                                                  observe_on_on_next{},
                                                  observe_on_on_error{},
                                                  observe_on_on_completed{},
                                                  dynamic_subscriber,
                                                  std::move(worker));
    }
};
} // namespace rpp::details
