// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
    if constexpr (memory_model == memory_model::use_stack)
        return container_wrapper<std::array<T, sizeof...(Ts)>>{std::forward<Ts>(items)...};
    else
        return std::make_shared<std::array<T, sizeof...(Ts)>>(std::array<T, sizeof...(Ts)>{std::forward<Ts>(items)...});
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
    return create<DT>([=, items = details::pack_variadic<memory_model, DT>(std::forward<T>(item), std::forward<Ts>(items)...)](const constraint::subscriber auto& subscriber)
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
