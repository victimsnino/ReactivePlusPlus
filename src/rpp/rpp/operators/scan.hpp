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

#include <rpp/subscribers/constraints.hpp>
#include <rpp/operators/fwd/scan.hpp>
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/utils/functors.hpp>

#include <rpp/defs.hpp>



#include <rpp/utils/utilities.hpp>

#include <memory>

IMPLEMENTATION_FILE (scan_tag);

namespace rpp::details
{
template<constraint::decayed_type Result, typename AccumulatorFn>
class scan_on_next
{
public:
    scan_on_next(const Result& seed, const AccumulatorFn& accumulator)
        : m_seed{seed}
        , m_accumulator{accumulator} {}

    void operator()(auto&& value, const rpp::constraint::subscriber auto& sub) const
    {
        m_seed = m_accumulator(std::move(m_seed), std::forward<decltype(value)>(value));
        sub.on_next(utils::as_const(m_seed));
    }

private:
    mutable Result                      m_seed;
    RPP_NO_UNIQUE_ADDRESS AccumulatorFn m_accumulator;
};

template<constraint::decayed_type Type, constraint::decayed_type Result, scan_accumulator<Result, Type> AccumulatorFn>
struct scan_impl
{
    Result                              initial_value;
    RPP_NO_UNIQUE_ADDRESS AccumulatorFn accumulator;

    template<constraint::subscriber_of_type<Result> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        // dynamic_state there to make shared_ptr for observer instead of making shared_ptr for state
        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          std::forward<TSub>(subscriber),
                                                          scan_on_next{initial_value, accumulator},
                                                          utils::forwarding_on_error{},
                                                          utils::forwarding_on_completed{});
    }
};
} // namespace rpp::details
