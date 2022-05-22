
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

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/subscribe_on.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/schedulers/fwd.hpp>

IMPLEMENTATION_FILE (subscribe_on_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs, schedulers::constraint::scheduler TScheduler>
auto subscribe_on_impl(TObs&& obs, const TScheduler& scheduler)
{
    return source::create<Type>([obs = std::forward<TObs>(obs), scheduler]<constraint::subscriber_of_type<Type> TSub>(TSub&& subscriber)
    {
        auto worker = scheduler.create_worker(subscriber.get_subscription());
        worker.schedule([obs, subscriber = std::forward<TSub>(subscriber)]() mutable ->schedulers::optional_duration
        {
            obs.subscribe(std::move(subscriber));
            return {};
        });
    });
}
} // namespace rpp::details
