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
 * \snippet from.cpp from_iterable
 *
 * \see https://reactivex.io/documentation/operators/from.html
 **/

#include <rpp/memory_model.hpp>
#include <rpp/schedulers/constraints.hpp>
#include <rpp/schedulers/immediate_scheduler.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/sources/fwd.hpp>
#include <rpp/utils/utilities.hpp>
#include <rpp/operators/map.hpp>

#include <array>
#include <ranges>
#include <type_traits>

namespace rpp::observable::details
{
template<typename T>
auto extract_iterable_from_packed(const T & v) -> const auto&
{
    if constexpr (std::ranges::range<T>)
        return v;
    else
        return *v;
}

template<typename ValueExtractStrategy = std::identity>
void iterate(const auto&                                   iterable,
             const schedulers::constraint::scheduler auto& scheduler,
             const constraint::subscriber auto&            subscriber)
{
    if constexpr (constraint::decayed_same_as<decltype(scheduler), schedulers::immediate>)
    {
        for (const auto& v : extract_iterable_from_packed(iterable))
        {
            if (subscriber.is_subscribed())
                subscriber.on_next(utils::as_const(v));
        }
        subscriber.on_completed();
    }
    else
    {
        auto worker = scheduler.create_worker(subscriber.get_subscription());
        worker.schedule([=, index = size_t{0}]() mutable-> schedulers::optional_duration
        {
            if (subscriber.is_subscribed())
            {
                const auto& extracted_iterable = extract_iterable_from_packed(iterable);
                const auto  end = std::cend(extracted_iterable);
                auto        itr = std::cbegin(extracted_iterable);

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
            }
            return schedulers::optional_duration{};
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

template<memory_model memory_model, constraint::decayed_type T, typename ...Ts>
auto pack_variadic(Ts&& ...items)
{
    return pack_to_container<memory_model, std::array<T, sizeof...(Ts)>>(std::forward<Ts>(items)...);
}

template<typename PackedIterable, schedulers::constraint::scheduler TScheduler>
class iterate_impl
{
public:
    iterate_impl(const PackedIterable& iterable, const TScheduler& scheduler)
        : m_iterable{iterable}
        , m_scheduler{scheduler} {}

    iterate_impl(PackedIterable&& iterable, const TScheduler& scheduler)
        : m_iterable{std::move(iterable)}
        , m_scheduler{scheduler} {}

    void operator()(const constraint::subscriber auto& subscriber) const
    {
        details::iterate(m_iterable, m_scheduler, subscriber);
    }

private:
    PackedIterable m_iterable;
    TScheduler     m_scheduler;
};
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
    return create<DT>(details::iterate_impl{details::pack_variadic<memory_model, DT>(std::forward<T>(item), std::forward<Ts>(items)...), scheduler });
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

/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable that emits a items from provided iterable
 * \tparam memory_model rpp::memory_model strategy used to handle provided iterable
 * \tparam TScheduler type of scheduler used for scheduling of submissions: next item will be submitted to scheduler when previous one is executed
 * \param iterable container with values which will be flattened
 *
 * \snippet from.cpp from_iterable
 * \snippet from.cpp from_iterable with model
 * \snippet from.cpp from_iterable with scheduler
 *
 * \see https://reactivex.io/documentation/operators/from.html
*/
template<memory_model memory_model, schedulers::constraint::scheduler TScheduler>
auto from_iterable(std::ranges::range auto&& iterable, const TScheduler& scheduler)
{
    using Container = std::decay_t<decltype(iterable)>;
    using Type = std::ranges::range_value_t<Container>;
    return create<Type>(details::iterate_impl{ details::pack_to_container<memory_model, Container>(std::forward<decltype(iterable)>(iterable)), scheduler });
}

/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable that calls provided callable and emits resulting value
 * \tparam memory_model rpp::memory_model strategy used to handle callable
 *
 * \snippet from.cpp from_callable
 *
 * \see https://reactivex.io/documentation/operators/from.html
*/
template<memory_model memory_model>
auto from_callable(std::invocable<> auto&& callable)
{
    auto obs = just<memory_model>(std::forward<decltype(callable)>(callable));

    if constexpr (std::same_as<std::invoke_result_t<decltype(callable)>, void>)
        return std::move(obs).map([](const auto& fn) { fn(); return utils::none{};});
    else
        return std::move(obs).map([](const auto& fn) { return fn(); });
}
} // namespace rpp::observable
