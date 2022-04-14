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
 * \brief This file contains implementation of `from` function to create rpp::specific_observable that constructed from provided items

 *
 * \see https://reactivex.io/documentation/operators/from.html
 **/

#include <rpp/memory_model.h>
#include <rpp/schedulers/constraints.h>
#include <rpp/schedulers/immediate_scheduler.h>
#include <rpp/sources/create.h>
#include <rpp/sources/fwd.h>
#include <rpp/utils/utilities.h>

#include <array>
#include <ranges>
#include <type_traits>

namespace rpp::observable::details
{
template<typename Rng>
concept DerefIterable = requires(Rng r)
{
    { *r } -> std::ranges::range;
};

void iterate(const DerefIterable auto&                     r,
             const schedulers::constraint::scheduler auto& scheduler,
             const constraint::subscriber auto&            subscriber)
{
    if constexpr (constraint::decayed_same_as<decltype(scheduler), schedulers::immediate>)
    {
        for (const auto& v : *r)
            subscriber.on_next(utilities::as_const(v));
        subscriber.on_completed();
    }
    else
    {
        auto worker = scheduler.create_worker(subscriber.get_subscription());
        worker.schedule([=, itr = (*r).cbegin()]() mutable-> schedulers::optional_duration
        {
            subscriber.on_next(utilities::as_const(*itr));
            if (++itr != (*r).cend())
                return schedulers::duration{}; // re-schedule this

            subscriber.on_completed();
            return {};
        });
    }
}

template<typename Cont>
struct container_wrapper
{
    const Cont& operator*() const { return container; }

    Cont container;
};

template<memory_model memory_model, constraint::decayed_type T, typename ...Ts>
auto pack_variadic(Ts&& ...items)
{
    if constexpr (memory_model == memory_model::use_stack)
        return container_wrapper<std::array<T, sizeof...(Ts)>>{ std::forward<Ts>(items)... };
    else
        return std::make_shared<std::array<T, sizeof...(Ts)>>(std::forward<Ts>(items)...);
}
} // namespace rpp::observable::details
namespace rpp::observable
{
/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable that emits a provided items and completes
 * \tparam memory_model rpp::memory_model startegy used to handle provided items.
 * \param scheduler type of scheduler used for scheduling of submissions
 * \param item first value to be sent
 * \param items rest values to be sent
 * \return rpp::specific_observable with provided items
 * 
 * \see https://reactivex.io/documentation/operators/from.html
 */
template<memory_model memory_model, typename T, typename ...Ts>
auto from(const schedulers::constraint::scheduler auto& scheduler, T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...)
{
    using DT = std::decay_t<T>;
    return create<DT>([=, items = details::pack_variadic<memory_model, DT>(std::forward<T>(item), std::forward<Ts>(items)...)](const constraint::subscriber auto& subscriber)
                      {
                          iterate(items, scheduler, subscriber);
                      });
}

/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable that emits a provided items and completes
 * \tparam memory_model rpp::memory_model startegy used to handle provided items.
 * \param item first value to be sent
 * \param items rest values to be sent
 * \return rpp::specific_observable with provided items
 *  
 * \see https://reactivex.io/documentation/operators/from.html
 */
template<memory_model memory_model, typename T, typename ...Ts>
auto from(T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...)
{
    return from<memory_model>(schedulers::immediate{}, std::forward<T>(item), std::forward<Ts>(items)...);
}
} // namespace rpp::observable
