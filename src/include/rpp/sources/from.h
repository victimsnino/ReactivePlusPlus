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
void iterate(const auto&                                   iterable,
             const schedulers::constraint::scheduler auto& scheduler,
             const constraint::subscriber auto&            subscriber)
{
    constexpr auto extract_iterable = []<typename T>(const T& v) -> const auto&
    {
        if constexpr (std::ranges::range<T>)
            return v;
        else
            return *v;
    };

    if constexpr (constraint::decayed_same_as<decltype(scheduler), schedulers::immediate>)
    {
        for (const auto& v : extract_iterable(iterable))
            subscriber.on_next(utilities::as_const(v));
        subscriber.on_completed();
    }
    else
    {
        auto worker = scheduler.create_worker(subscriber.get_subscription());
        worker.schedule([=, index = size_t{0}]() mutable-> schedulers::optional_duration
        {
            const auto end = std::cend(extract_iterable(iterable));
            auto       itr = std::cbegin(extract_iterable(iterable));

            std::ranges::advance(itr, index, end);

            if (itr != end)
            {
                subscriber.on_next(utilities::as_const(*itr));
                if (std::next(itr) != end) // it was not last
                {
                    ++index;
                    return schedulers::duration{}; // re-schedule this
                }
            }

            subscriber.on_completed();
            return {};
        });
    }
}

template<memory_model memory_model, std::ranges::range Container, typename ...Ts>
auto pack_to_container(Ts&& ...items)
{
    if constexpr (memory_model == memory_model::use_stack)
        return Container{std::forward<Ts>(items)...};
    else
        // raw new call to avoid extra copy/moves for items
        return std::shared_ptr<Container>(new Container{std::forward<Ts>(items)...});
}
} // namespace rpp::observable::details

namespace rpp::observable
{
/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable that emits a items from provided iterable
 * \tparam memory_model rpp::memory_model startegy used to handle provided iterable
 * \tparam TScheduler type of scheduler used for scheduling of submissions: next item will be submitted to scheduler when previous one is executed
 * \param iterable container with values which will be flattened
 * \return 
 */
template<memory_model memory_model, schedulers::constraint::scheduler TScheduler>
auto from(std::ranges::range auto&& iterable, const TScheduler& scheduler)
{
    using Container = std::decay_t<decltype(iterable)>;
    using Type = std::ranges::range_value_t<Container>;
    return create<Type>([=, items = details::pack_to_container<memory_model, Container>(std::forward<decltype(iterable)>(iterable))](const constraint::subscriber auto& subscriber)
    {
        details::iterate(items, scheduler, subscriber);
    });
}
} // namespace rpp::observable
