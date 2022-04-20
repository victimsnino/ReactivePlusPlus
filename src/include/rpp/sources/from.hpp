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
 * \brief This file contains implementation of `from` function to create rpp::specific_observable that constructed from provided items
 * \snippet from.cpp from
 * \snippet from.cpp from with model
 * \snippet from.cpp from with scheduler
 *
 * \see https://reactivex.io/documentation/operators/from.hpptml
 **/

#include <rpp/memory_model.hpp>
#include <rpp/schedulers/constraints.hpp>
#include <rpp/schedulers/immediate_scheduler.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/sources/fwd.hpp>
#include <rpp/utils/utilities.hpp>

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
            subscriber.on_next(utils::as_const(v));
        subscriber.on_completed();
    }
    else
    {
        auto worker = scheduler.create_worker(subscriber.get_subscription());
        worker.schedule([=, index = size_t{0}]() mutable-> schedulers::optional_duration
        {
            const auto end = std::cend(extract_iterable(iterable));
            auto       itr = std::cbegin(extract_iterable(iterable));

            std::ranges::advance(itr, static_cast<int64_t>(index), end);

            if (itr != end)
            {
                subscriber.on_next(utils::as_const(*itr));
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
 *
 * \snippet from.cpp from
 * \snippet from.cpp from with model
 * \snippet from.cpp from with scheduler
 *
 * \see https://reactivex.io/documentation/operators/from.hpptml
*/
template<memory_model memory_model, schedulers::constraint::scheduler TScheduler>
auto from(std::ranges::range auto&& iterable, const TScheduler& scheduler)
{
    using Container = std::decay_t<decltype(iterable)>;
    using Type = std::ranges::range_value_t<Container>;
    return create<Type>([=, items = details::pack_to_container<memory_model, Container>(std::forward<decltype(iterable)>(iterable))](const constraint::subscriber_of_type<Type> auto& subscriber)
    {
        details::iterate(items, scheduler, subscriber);
    });
}
} // namespace rpp::observable
