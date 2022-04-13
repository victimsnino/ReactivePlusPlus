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
#include <rpp/schedulers/immediate_scheduler.h>
#include <rpp/sources/create.h>
#include <rpp/sources/fwd.h>

#include <type_traits>

namespace rpp::observable::details
{
template<rpp::schedulers::constraint::scheduler Scheduler = rpp::schedulers::immediate>
auto just_use_stack(auto&& item, const Scheduler& scheduler)
{
    return create<std::decay_t<decltype(item)>>([=, item = std::forward<decltype(item)>(item)](const constraint::subscriber auto& subscriber)
    {
        if constexpr (std::is_same_v<Scheduler, rpp::schedulers::immediate>)
        {
            subscriber.on_next(item);
            subscriber.on_completed();
        }
        else
        {
            auto worker = scheduler.create_worker(subscriber.get_subscription());
            worker.schedule([subscriber, item]() mutable -> rpp::schedulers::optional_duration
                {
                    subscriber.on_next(std::move(item));
                    subscriber.on_completed();
                    return {};
                });
        }
    });
}

template<typename T, rpp::schedulers::constraint::scheduler Scheduler = rpp::schedulers::immediate>
auto just_use_shared(T&& item, const Scheduler& scheduler)
{
    auto as_shared = std::make_shared<std::decay_t<T>>(std::forward<T>(item));
    return create<std::decay_t<T>>([=](const constraint::subscriber auto& subscriber)
        {
            auto worker = scheduler.create_worker(subscriber.get_subscription());
            worker.schedule([=]() mutable -> rpp::schedulers::optional_duration
                {
                    subscriber.on_next(*as_shared);
                    subscriber.on_completed();
                    return {};
                });
        });
}
} // namespace rpp::observable::details
namespace rpp::observable
{
/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable that emits a particular item and completes
 * \tparam memory_model rpp::memory_model startegy used to handle item.
 * \tparam Scheduler type of scheduler used for scheduling of submissions
 * \param item value to be sent
 * \return rpp::specific_observable with provided item
 *
 * \snippet just.cpp just
 * \snippet just.cpp just memory model
 * \snippet just.cpp just scheduler
 *
 * \see https://reactivex.io/documentation/operators/just.html
 */
template<memory_model memory_model, rpp::schedulers::constraint::scheduler Scheduler>
auto just(auto&& item, const Scheduler& scheduler)
{
    if constexpr (memory_model == memory_model::use_stack)
        return details::just_use_stack(std::forward<decltype(item)>(item), scheduler);
    else
        return details::just_use_shared(std::forward<decltype(item)>(item), scheduler);
}
} // namespace rpp::observable
