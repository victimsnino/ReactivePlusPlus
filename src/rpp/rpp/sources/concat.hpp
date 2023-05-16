//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include "rpp/utils/constraints.hpp"
#include <rpp/sources/fwd.hpp>
#include <rpp/sources/from.hpp>
#include <rpp/observables/base_observable.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/disposables/base_disposable.hpp>
#include <rpp/memory_model.hpp>


namespace rpp::details
{
template<constraint::memory_model memory_model, rpp::constraint::observable TObservable, rpp::constraint::observable ...TObservables>
auto pack_observables(TObservable&& obs, TObservables&&...others)
{
    if constexpr ((rpp::constraint::decayed_same_as<TObservable, TObservables> && ...))
    {
        return pack_variadic<memory_model, std::decay_t<TObservable>>(std::forward<TObservable>(obs), std::forward<TObservables>(others)...);
    }
    else
    {
        return pack_variadic<memory_model, dynamic_observable<utils::extract_observable_type_t<TObservable>>>(std::forward<TObservable>(obs).as_dynamic(), std::forward<TObservables>(others).as_dynamic()...);
    }
}

template<constraint::decayed_type PackedContainer>
struct concat_strategy;

template<constraint::decayed_type PackedContainer>
struct concat_source_observer_strategy
{
    using Type = utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;

    RPP_NO_UNIQUE_ADDRESS mutable PackedContainer container;

    constexpr static operators::details::forwarding_on_next_strategy on_next{};
    constexpr static operators::details::forwarding_on_error_strategy on_error{};
    constexpr static operators::details::forwarding_set_upstream_strategy set_upstream{};
    constexpr static operators::details::forwarding_is_disposed_strategy is_disposed{};

    void on_completed(rpp::constraint::observer auto& observer) const
    {
        container.increment_iterator();
        concat_strategy<PackedContainer>::drain(std::move(container), std::move(observer));
    }
};

template<constraint::decayed_type PackedContainer>
struct concat_strategy
{
    RPP_NO_UNIQUE_ADDRESS PackedContainer container;

    using Type = utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;

    template<constraint::observer_strategy<Type> Strategy>
    void subscribe(base_observer<Type, Strategy>&& observer) const
    {
        drain(container, std::move(observer));
    }

    template<constraint::observer_strategy<Type> Strategy>
    static void drain(constraint::decayed_same_as<PackedContainer> auto&& container, base_observer<Type, Strategy>&& observer)
    {
        if (const auto itr = container.get_actual_iterator(); itr != std::cend(container))
        {
            decltype(auto) observable = PackedContainer::extract_value_from_itr(itr);
            observable.subscribe(base_observer<Type,
                                               rpp::operators::details::operator_strategy_base<Type, base_observer<Type, Strategy>,
                                               concat_source_observer_strategy<PackedContainer>>>
                                               {
                                                   std::move(observer),
                                                   std::forward<decltype(container)>(container)
                                               });
        }
        else
        {
            observer.on_completed();
        }
    }
};

template<typename PackedContainer>
auto make_concat_from_iterable(PackedContainer&& container)
{
    return base_observable<utils::extract_observable_type_t<utils::iterable_value_t<std::decay_t<PackedContainer>>>,
                           concat_strategy<std::decay_t<PackedContainer>>>{std::forward<PackedContainer>(container)};
}
} // namespace rpp::details
namespace rpp::source
{
/**
 * @brief Make observable which would merge emissions from underlying observables but without overlapping (current observable completes THEN next started to emit its values)
 *
 * @marble concat
 {
     source observable :
     {
         +--1-2-3-|
         .....+4--6-|
     }
     operator "concat" : +--1-2-3-4--6-|
 }
 *
 * @details Actually it subscribes on first observable from emissions. When first observable completes, then it subscribes on second observable from emissions and etc...
 *
 * @param obs first observalbe to subscribe on
 * @param others rest list of observables to subscribe on
 * @tparam memory_model rpp::memory_model strategy used to handle provided observables
 *
 * @return new base_observvable with the concat operator as most recent operator.
 * @warning #include <rpp/operators/concat.hpp>
 *
 * @par Example
 * @snippet concat.cpp concat_as_source
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/concat.html
 */
template<constraint::memory_model memory_model /*= memory_model::use_stack*/, rpp::constraint::observable TObservable, rpp::constraint::observable ...TObservables>
    requires (std::same_as<rpp::utils::extract_observable_type_t<TObservable>, rpp::utils::extract_observable_type_t<TObservables>> && ...)
auto concat(TObservable&& obs, TObservables&&...others)
{
    return make_concat_from_iterable(pack_observables<memory_model>(std::forward<TObservable>(obs), std::forward<TObservables>(others)...));
}

/**
 * @brief Make observable which would merge emissions from underlying observables but without overlapping (current observable completes THEN next started to emit its values)
 *
 * @marble concat
 {
     source observable :
     {
         +--1-2-3-|
         .....+4--6-|
     }
     operator "concat" : +--1-2-3-4--6-|
 }
 *
 * @details Actually it subscribes on first observable from emissions. When first observable completes, then it subscribes on second observable from emissions and etc...
 *
 * @iterable is container with observables to subscribe on
 * @tparam memory_model rpp::memory_model strategy used to handle provided observables
 * @return new base_observvable with the concat operator as most recent operator.
 * @warning #include <rpp/operators/concat.hpp>
 *
 * @par Example
 * @snippet concat.cpp concat_as_source_vector
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/concat.html
 */
template<constraint::memory_model memory_model /*= memory_model::use_stack*/, constraint::iterable Iterable>
    requires constraint::observable<utils::iterable_value_t<Iterable>>
auto concat(Iterable&& iterable)
{
    return make_concat_from_iterable(details::pack_to_container<memory_model, std::decay_t<Iterable>>(std::forward<Iterable>(iterable)));
}
}