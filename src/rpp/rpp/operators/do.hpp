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
#include <rpp/subscribers/constraints.hpp>
#include <rpp/operators/fwd/do.hpp>


IMPLEMENTATION_FILE(do_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observer_of_type<Type> TObs>
struct do_impl
{
    TObs observer;

    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();

        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  [](auto&& value, const TSub& subscriber, const TObs& do_observer)
                                                  {
                                                      do_observer.on_next(utils::as_const(value));
                                                      subscriber.on_next(std::forward<decltype(value)>(value));
                                                  },
                                                  [](const std::exception_ptr& err,
                                                     const TSub&               subscriber,
                                                     const TObs&               do_observer)
                                                  {
                                                      do_observer.on_error(err);
                                                      subscriber.on_error(err);
                                                  },
                                                  [](const TSub& subscriber, const TObs& do_observer)
                                                  {
                                                      do_observer.on_completed();
                                                      subscriber.on_completed();
                                                  },
                                                  std::forward<TSub>(subscriber),
                                                  observer);
    }
};
} // namespace rpp::details
