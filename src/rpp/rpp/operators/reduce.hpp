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

#include <rpp/defs.hpp>                                    // RPP_NO_UNIQUE_ADDRESS
#include <rpp/operators/lift.hpp>                          // required due to operator uses lift
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/reduce.hpp>                      // own forwarding
#include <rpp/subscribers/constraints.hpp>                 // constraint::subscriber
#include <rpp/utils/functors.hpp>                          // forwarding_on_error
#include <rpp/utils/utilities.hpp>                         // utils::as_const


IMPLEMENTATION_FILE(reduce_tag);

namespace rpp::details
{
template<constraint::decayed_type Result, typename AccumulatorFn>
struct reduce_state
{
    mutable Result                      seed;
    RPP_NO_UNIQUE_ADDRESS AccumulatorFn accumulator;
};

struct reduce_on_next
{
    template<constraint::decayed_type Result, typename AccumulatorFn>
    void operator()(auto&&                                   value,
                    const constraint::subscriber auto&,
                    const reduce_state<Result, AccumulatorFn>& state) const
    {
        state.seed = state.accumulator(std::move(state.seed), std::forward<decltype(value)>(value));
    }
};

struct reduce_on_completed
{
    template<constraint::decayed_type Result, typename AccumulatorFn>
    void operator()(const constraint::subscriber auto&       sub,
                    const reduce_state<Result, AccumulatorFn>& state) const
    {
        sub.on_next(std::move(state.seed));
        sub.on_completed();
    }
};

template<constraint::decayed_type Type, constraint::decayed_type Result, reduce_accumulator<Result, Type> AccumulatorFn>
struct reduce_impl
{
    Result                              initial_value;
    RPP_NO_UNIQUE_ADDRESS AccumulatorFn accumulator;

    template<constraint::subscriber_of_type<Result> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        // dynamic_state there to make shared_ptr for observer instead of making shared_ptr for state
        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          reduce_on_next{},
                                                          utils::forwarding_on_error{},
                                                          reduce_on_completed{},
                                                          std::forward<TSub>(subscriber),
                                                          reduce_state<Result, AccumulatorFn>{initial_value, accumulator});
    }
};
} // namespace rpp::details

