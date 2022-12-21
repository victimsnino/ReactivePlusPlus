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
#include <rpp/observables/constraints.hpp>                 
#include <rpp/operators/lift.hpp>                          // required due to operator uses lift
#include <rpp/operators/details/subscriber_with_state.hpp> // create_subscriber_with_state
#include <rpp/operators/fwd/reduce.hpp>                      // own forwarding
#include <rpp/subscribers/constraints.hpp>                 // constraint::subscriber
#include <rpp/utils/functors.hpp>                          // forwarding_on_error
#include <rpp/utils/utilities.hpp>                         // utils::as_const


IMPLEMENTATION_FILE(reduce_tag);

namespace rpp::details
{
template<constraint::decayed_type Seed, typename AccumulatorFn, std::invocable<Seed&&> SelectorFn = std::identity>
struct reduce_state
{
    mutable Seed                        seed;
    RPP_NO_UNIQUE_ADDRESS AccumulatorFn accumulator;
    RPP_NO_UNIQUE_ADDRESS SelectorFn    selector{};
};

struct reduce_on_next
{
    template<constraint::decayed_type Result, typename AccumulatorFn, typename SelectorFn>
    void operator()(auto&&                                   value,
                    const constraint::subscriber auto&,
                    const reduce_state<Result, AccumulatorFn, SelectorFn>& state) const
    {
        state.seed = state.accumulator(std::move(state.seed), std::forward<decltype(value)>(value));
    }
};

struct reduce_on_completed
{
    template<constraint::decayed_type Result, typename AccumulatorFn, typename SelectorFn>
    void operator()(const constraint::subscriber auto&                     sub,
                    const reduce_state<Result, AccumulatorFn, SelectorFn>& state) const
    {
        try
        {
            sub.on_next(state.selector(std::move(state.seed)));
        }
        catch (...)
        {
            sub.on_error(std::current_exception());
            return;
        }
        sub.on_completed();
    }
};

template<constraint::decayed_type Type, constraint::decayed_type Seed, reduce_accumulator<Seed, Type> AccumulatorFn, std::invocable<Seed&&> ResultSelectorFn>
struct reduce_impl
{
    Seed                                   initial_value;
    RPP_NO_UNIQUE_ADDRESS AccumulatorFn    accumulator;
    RPP_NO_UNIQUE_ADDRESS ResultSelectorFn selector;

    template<constraint::subscriber_of_type<utils::decayed_invoke_result_t<ResultSelectorFn, Seed>> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();
        // dynamic_state there to make shared_ptr for observer instead of making shared_ptr for state
        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          reduce_on_next{},
                                                          utils::forwarding_on_error{},
                                                          reduce_on_completed{},
                                                          std::forward<TSub>(subscriber),
                                                          reduce_state<Seed, AccumulatorFn, ResultSelectorFn>{initial_value, accumulator, selector});
    }
};

template<constraint::decayed_type CastBeforeDivide, constraint::observable TObs>
auto average_impl(TObs&& observable)
{
    using Type = utils::extract_observable_type_t<std::decay_t<TObs>>;
    using Pair = std::pair<std::optional<Type>, int32_t>;
    return std::forward<TObs>(observable).reduce(Pair{},
                                                 [](Pair&& seed, auto&& val)
                                                 {
                                                     if (seed.first)
                                                        seed.first.value() += std::forward<decltype(val)>(val);
                                                     else
                                                         seed.first = std::forward<decltype(val)>(val);
                                                     ++seed.second;
                                                     return std::move(seed);
                                                 },
                                                 [](Pair&& seed)
                                                 {
                                                     if (!seed.first)
                                                        throw utils::not_enough_emissions{"`average` operator requires at least one emission to calculate average"};

                                                     return static_cast<CastBeforeDivide>(std::move(seed.first).value()) / seed.second;
                                                 });
}

template<constraint::observable TObs>
auto sum_impl(TObs&& observable)
{
    using Type = utils::extract_observable_type_t<std::decay_t<TObs>>;
    return std::forward<TObs>(observable).reduce(std::optional<Type>{},
                                                 [](std::optional<Type>&& seed, auto&& val)
                                                 {
                                                     if (!seed)
                                                         seed = std::forward<decltype(val)>(val);
                                                     else
                                                        seed.value() += std::forward<decltype(val)>(val);
                                                     return std::move(seed);
                                                 },
                                                 [](std::optional<Type>&& seed)
                                                 {
                                                     if (!seed)
                                                         throw utils::not_enough_emissions{"`sum` operator requires at least one emission to calculate sum"};

                                                     return std::move(seed.value());
                                                 });
}

template<constraint::observable TObs>
auto count_impl(TObs&& observable)
{
    return std::forward<TObs>(observable).reduce(size_t{}, [](size_t seed, auto&&) { return ++seed; });
}

template<constraint::observable TObs, typename Comparator>
auto min_impl(TObs&& observable, Comparator&& comparator)
{
    using Type = utils::extract_observable_type_t<std::decay_t<TObs>>;
    return std::forward<TObs>(observable).reduce(std::optional<Type>{},
                                                 [comparator](std::optional<Type>&& seed, auto&& val)
                                                 {
                                                     if (!seed || comparator(utils::as_const(val), seed.value()))
                                                         seed = std::forward<decltype(val)>(val);
                                                     return std::move(seed);
                                                 },
                                                 [](std::optional<Type>&& seed)
                                                 {
                                                     if (!seed)
                                                         throw utils::not_enough_emissions{"`min` operator requires at least one emission to calculate min"};

                                                     return std::move(seed.value());
                                                 });
}

template<constraint::observable TObs, typename Comparator>
auto max_impl(TObs&& observable, Comparator&& comparator)
{
    using Type = utils::extract_observable_type_t<std::decay_t<TObs>>;
    return std::forward<TObs>(observable).reduce(std::optional<Type>{},
                                                 [comparator](std::optional<Type>&& seed, auto&& val)
                                                 {
                                                     if (!seed || comparator(seed.value(), utils::as_const(val)))
                                                         seed = std::forward<decltype(val)>(val);
                                                     return std::move(seed);
                                                 },
                                                 [](std::optional<Type>&& seed)
                                                 {
                                                     if (!seed)
                                                         throw utils::not_enough_emissions{"`max` operator requires at least one emission to calculate min"};

                                                     return std::move(seed.value());
                                                 });
}
} // namespace rpp::details

