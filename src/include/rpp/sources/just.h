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

/**
 * \file
 * \brief This file contains implementation of `just` function to create rpp::specific_observable that emits a particular item
 * \snippet just.cpp just
 * \snippet just.cpp just memory model
 * \snippet just.cpp just scheduler
 *
 * \see https://reactivex.io/documentation/operators/just.html
 **/

#include <rpp/memory_model.h>
#include <rpp/schedulers/constraints.h>
#include <rpp/sources/from.h>
#include <rpp/sources/fwd.h>

#include <array>
#include <ranges>
#include <memory>
#include <type_traits>

namespace rpp::observable::details
{
template<memory_model memory_model, constraint::decayed_type T, typename ...Ts>
auto pack_variadic(Ts&& ...items)
{
    return pack_to_container<memory_model, std::array<T, sizeof...(Ts)>>(std::forward<Ts>(items)...);
}
} // namespace rpp::observable::details

namespace rpp::observable
{
/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable that emits a particular items and completes
 * \tparam memory_model rpp::memory_model startegy used to handle provided items
 * \tparam Scheduler type of scheduler used for scheduling of submissions: next item will be submitted to scheduler when previous one is executed
 * \param item first value to be sent
 * \param items rest values to be sent
 * \return rpp::specific_observable with provided item
 *
 * \snippet just.cpp just
 * \snippet just.cpp just memory model
 * \snippet just.cpp just scheduler
 *
 * \see https://reactivex.io/documentation/operators/just.html
 */
template<memory_model memory_model, typename T, typename ...Ts>
auto just(const schedulers::constraint::scheduler auto& scheduler, T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...)
{
    using DT = std::decay_t<T>;
    return create<DT>([=, items = details::pack_variadic<memory_model, DT>(std::forward<T>(item), std::forward<Ts>(items)...)](const constraint::subscriber_of_type<DT> auto& subscriber)
    {
        details::iterate(items, scheduler, subscriber);
    });
}

/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable that emits a particular items and completes
 * \details this overloading uses immediate scheduler as default
 * \tparam memory_model rpp::memory_model startegy used to handle provided items
 * \param item first value to be sent
 * \param items rest values to be sent
 * \return rpp::specific_observable with provided item
 *
 * \snippet just.cpp just
 * \snippet just.cpp just memory model
 * \snippet just.cpp just scheduler
 *
 * \see https://reactivex.io/documentation/operators/just.html
 */
template<memory_model memory_model, typename T, typename ...Ts>
auto just(T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...)
{
    return just<memory_model>(schedulers::immediate{}, std::forward<T>(item), std::forward<Ts>(items)...);
}
} // namespace rpp::observable
