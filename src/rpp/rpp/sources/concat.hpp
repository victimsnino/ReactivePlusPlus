//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/sources/fwd.hpp>
#include <rpp/sources/from.hpp>
#include <rpp/observables/observable.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/memory_model.hpp>
#include <exception>
#include <optional>


namespace rpp::details
{
template<constraint::decayed_type PackedContainer>
struct concat_strategy;

template<rpp::constraint::observer TObserver, constraint::decayed_type PackedContainer>
struct concat_source_observer_strategy
{
    using ValueType = rpp::utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;

    RPP_NO_UNIQUE_ADDRESS mutable TObserver       observer;
    RPP_NO_UNIQUE_ADDRESS mutable PackedContainer container;
    mutable size_t                                index;

    template<typename T>
    void on_next(T&& v) const                          { observer.on_next(std::forward<T>(v)); }
    void on_error(const std::exception_ptr& err) const { observer.on_error(err); }

    void set_upstream(const disposable_wrapper& d)     { observer.set_upstream(d); }
    bool is_disposed() const                           { return observer.is_disposed(); }

    void on_completed() const
    {
        concat_strategy<PackedContainer>::drain(std::move(container), index, std::move(observer));
    }
};

template<constraint::decayed_type PackedContainer>
struct concat_strategy
{
    template<typename...Args>
        requires (!constraint::variadic_decayed_same_as<concat_strategy<PackedContainer>, Args...>)
    concat_strategy(Args&&...args)
        : container{std::forward<Args>(args)...}
    {}

    RPP_NO_UNIQUE_ADDRESS PackedContainer container;

    using ValueType = rpp::utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;

    template<constraint::observer_strategy<ValueType> Strategy>
    void subscribe(observer<ValueType, Strategy>&& obs) const
    {
        drain(container, size_t{}, std::move(obs));
    }

    template<constraint::observer_strategy<ValueType> Strategy>
    static void drain(constraint::decayed_same_as<PackedContainer> auto&& container, size_t index, observer<ValueType, Strategy>&& obs)
    {
        std::optional<utils::iterable_value_t<PackedContainer>> observable{};
        bool is_last_observable = false;
        try
        {
            auto itr = std::cbegin(container);
            std::advance(itr, static_cast<int64_t>(index++));

            if (itr != std::cend(container))
            {
                observable.emplace(*itr);
                is_last_observable = std::next(itr) == std::cend(container);
            }

        }
        catch (...)
        {
            obs.on_error(std::current_exception());
            return;
        }

        if (!observable.has_value())
        {
            obs.on_completed();
            return;
        }

        if (is_last_observable)
            observable->subscribe(std::move(obs));
        else
            observable->subscribe(observer<ValueType, concat_source_observer_strategy<observer<ValueType, Strategy>,PackedContainer>>{std::move(obs), std::forward<decltype(container)>(container), index});
    }
};

template<typename PackedContainer, typename ...Args>
auto make_concat_from_iterable(Args&& ...args)
{
    return observable<utils::extract_observable_type_t<utils::iterable_value_t<std::decay_t<PackedContainer>>>,
                           concat_strategy<std::decay_t<PackedContainer>>>{std::forward<Args>(args)...};
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
    if constexpr ((rpp::constraint::decayed_same_as<TObservable, TObservables> && ...))
    {
        using inner_container = std::array<std::decay_t<TObservable>, sizeof...(TObservables) + 1>;
        using container = std::conditional_t<std::same_as<memory_model, rpp::memory_model::use_stack>, inner_container, details::shared_container<inner_container>>;
        return rpp::details::make_concat_from_iterable<container>(std::forward<TObservable>(obs), std::forward<TObservables>(others)...);
    }
    else
        return concat<memory_model>(std::forward<TObservable>(obs).as_dynamic(), std::forward<TObservables>(others).as_dynamic()...);
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
    using container = std::conditional_t<std::same_as<memory_model, rpp::memory_model::use_stack>, std::decay_t<Iterable>, details::shared_container<std::decay_t<Iterable>>>;
    return rpp::details::make_concat_from_iterable<container>(std::forward<Iterable>(iterable));
}
}
