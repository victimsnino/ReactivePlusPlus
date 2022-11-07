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

#include <rpp/memory_model.hpp>
#include <rpp/schedulers/immediate_scheduler.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/sources/fwd.hpp>
#include <rpp/utils/utilities.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/utils/function_traits.hpp>
#include <rpp/defs.hpp>


#include <array>
#include <ranges>
#include <type_traits>

IMPLEMENTATION_FILE(just_tag);
IMPLEMENTATION_FILE(from_tag);

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

void iterate(auto&&                                        iterable,
             const schedulers::constraint::scheduler auto& scheduler,
             constraint::subscriber auto&&                 subscriber)
{
    if constexpr (constraint::decayed_same_as<decltype(scheduler), schedulers::immediate>)
    {
        for (const auto& v : extract_iterable_from_packed(iterable))
        {
            if (subscriber.is_subscribed())
                subscriber.on_next(utils::as_const(v));
            else
                return;
        }
        subscriber.on_completed();
    }
    else
    {
        auto worker = scheduler.create_worker(subscriber.get_subscription());
        worker.schedule([iterable=std::forward<decltype(iterable)>(iterable), 
                         subscriber=std::forward<decltype(subscriber)>(subscriber), 
                         index = size_t{0}]() mutable-> schedulers::optional_duration
        {            
            try
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
            catch(...)
            {
                subscriber.on_error(std::current_exception());
            }
            return std::nullopt;

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

    template<constraint::subscriber TSub>
    void operator()(TSub&& subscriber) const &
    {
        details::iterate(m_iterable, m_scheduler, std::forward<TSub>(subscriber));
    }

    template<constraint::subscriber TSub>
    void operator()(TSub&& subscriber) const &&
    {
        details::iterate(std::move(m_iterable), m_scheduler, std::forward<TSub>(subscriber));
    }

private:
    mutable PackedIterable           m_iterable;
    RPP_NO_UNIQUE_ADDRESS TScheduler m_scheduler;
};
} // namespace rpp::observable::details

namespace rpp::observable
{
/**
 * \brief Creates rpp::specific_observable that emits a particular items and completes
 * 
 * \marble just
   {
       operator "just(1,2,3,5)": +-1-2-3-5-|
   }
 *
 * \tparam memory_model rpp::memory_model startegy used to handle provided items
 * \tparam Scheduler type of scheduler used for scheduling of submissions: next item will be submitted to scheduler when previous one is executed
 * \param item first value to be sent
 * \param items rest values to be sent
 * \return rpp::specific_observable with provided item
 *
 * \par Examples:
 * \snippet just.cpp just
 * \snippet just.cpp just memory model
 * \snippet just.cpp just scheduler
 *
 * \ingroup creational_operators
 * \see https://reactivex.io/documentation/operators/just.html
 */
template<memory_model memory_model /* = memory_model::use_stack */, typename T, typename ...Ts>
auto just(const schedulers::constraint::scheduler auto& scheduler, T&& item, Ts&& ...items) requires (rpp::details::is_header_included<rpp::details::just_tag, T, Ts...> && (constraint::decayed_same_as<T, Ts> && ...))
{
    return create<std::decay_t<T>>(details::iterate_impl{details::pack_variadic<memory_model, std::decay_t<T>>(std::forward<T>(item), std::forward<Ts>(items)...), scheduler });
}

/**
 * \brief Creates rpp::specific_observable that emits a particular items and completes
 * \warning this overloading uses immediate scheduler as default
 * 
 * \marble just
   {
       operator "just(1,2,3,5)": +-1-2-3-5-|
   }
 * 
 * \tparam memory_model rpp::memory_model startegy used to handle provided items
 * \param item first value to be sent
 * \param items rest values to be sent
 * \return rpp::specific_observable with provided item
 *
 * \par Examples:
 * \snippet just.cpp just
 * \snippet just.cpp just memory model
 * \snippet just.cpp just scheduler
 *
 * \ingroup creational_operators
 * \see https://reactivex.io/documentation/operators/just.html
 */
template<memory_model memory_model /* = memory_model::use_stack */, typename T, typename ...Ts>
auto just(T&& item, Ts&& ...items) requires (rpp::details::is_header_included<rpp::details::just_tag, T, Ts...> && (constraint::decayed_same_as<T, Ts> && ...))
{
    return just<memory_model>(schedulers::immediate{}, std::forward<T>(item), std::forward<Ts>(items)...);
}

/**
 * \brief Creates rpp::specific_observable that emits a items from provided iterable
 * 
 * \marble from_iterable
   {
       operator "from_iterable({1,2,3,5})": +-1-2-3-5-|
   }
 *
 * \tparam memory_model rpp::memory_model strategy used to handle provided iterable
 * \param scheduler is scheduler used for scheduling of submissions: next item will be submitted to scheduler when previous one is executed
 * \param iterable container with values which will be flattened
 *
 * \par Examples:
 * \snippet from.cpp from_iterable
 * \snippet from.cpp from_iterable with model
 * \snippet from.cpp from_iterable with scheduler
 *
 * \ingroup creational_operators
 * \see https://reactivex.io/documentation/operators/from.html
*/
template<memory_model memory_model /* = memory_model::use_stack */, schedulers::constraint::scheduler TScheduler /* = schedulers::immediate */>
auto from_iterable(std::ranges::range auto&& iterable, const TScheduler& scheduler /* = TScheduler{} */) requires rpp::details::is_header_included<rpp::details::from_tag, TScheduler >
{
    using Container = std::decay_t<decltype(iterable)>;
    return create<std::ranges::range_value_t<Container>>(details::iterate_impl{ details::pack_to_container<memory_model, Container>(std::forward<decltype(iterable)>(iterable)), scheduler });
}

/**
 * \brief Creates rpp::specific_observable that calls provided callable and emits resulting value of this callable
 * 
 * \marble from_callable
   {
       operator "from_callable: [](){return 42;}": +-(42)--|
   }
 *
 * \tparam memory_model rpp::memory_model strategy used to handle callable
 *
 * \par Example
 * \snippet from.cpp from_callable
 *
 * \ingroup creational_operators
 * \see https://reactivex.io/documentation/operators/from.html
*/
template<memory_model memory_model /* = memory_model::use_stack */>
auto from_callable(std::invocable<> auto&& callable) requires rpp::details::is_header_included<rpp::details::from_tag, decltype(callable)>
{
    auto obs = just<memory_model>(std::forward<decltype(callable)>(callable));

    if constexpr (std::same_as<utils::decayed_invoke_result_t<decltype(callable)>, void>)
        return std::move(obs).map([](const auto& fn) { fn(); return utils::none{};});
    else
        return std::move(obs).map([](const auto& fn) { return fn(); });
}
} // namespace rpp::observable
