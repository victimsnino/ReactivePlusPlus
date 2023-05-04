//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include "rpp/memory_model.hpp"
#include "rpp/observables/fwd.hpp"
#include <rpp/sources/fwd.hpp>
#include <rpp/sources/from.hpp>
#include <rpp/observables/base_observable.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/disposables/base_disposable.hpp>

#include <memory>
#include <utility>

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
class concat_disposable;

template<constraint::decayed_type PackedContainer, constraint::observer_strategy<utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>> Strategy>
struct concat_source_observer_strategy
{
    using Type = utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;

    mutable base_observer<Type, Strategy>               observer;
    std::shared_ptr<concat_disposable<PackedContainer>> disposable;

    void set_upstream(const disposable_wrapper& d)     { disposable->add(d.get_original()); }
    bool is_disposed() const                           { return observer.is_disposed(); }

    void on_next(const Type& v) const                  { observer.on_next(v); }
    void on_next(Type&& v) const                       { observer.on_next(std::move(v)); }
    void on_error(const std::exception_ptr& err) const { observer.on_error(err); }

    void on_completed() const
    {
        disposable->drain(std::move(observer));
    }
};


template<constraint::decayed_type PackedContainer>
class concat_disposable final : public base_disposable, public std::enable_shared_from_this<concat_disposable<PackedContainer>>
{
    using Type = utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;

public:
    explicit concat_disposable(const PackedContainer& container) : m_container{container} {}

    template<constraint::observer_strategy<Type> Strategy>
    void drain(base_observer<Type, Strategy>&& observer)
    {
        if (const auto itr = m_container.get_actual_iterator(); itr != std::cend(m_container))
        {
            m_container.increment_iterator();
            itr->subscribe(base_observer<Type, concat_source_observer_strategy<PackedContainer, Strategy>>{std::move(observer), this->shared_from_this()});
        } 
        else 
        {
            observer.on_completed();
        }
    }

private:
    RPP_NO_UNIQUE_ADDRESS PackedContainer m_container;
};

template<constraint::decayed_type PackedContainer>
struct concat_strategy
{
    RPP_NO_UNIQUE_ADDRESS PackedContainer container;

    using Type = utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;

    template<constraint::observer_strategy<Type> Strategy>
    void subscribe(base_observer<Type, Strategy>&& observer) const
    {
        auto disposable = std::make_shared<concat_disposable<PackedContainer>>(container);
        
        observer.set_upstream(disposable_wrapper{disposable});
        disposable->drain(std::move(observer));
    }
};

template<typename PackedContainer>
auto make_concat_from_iterable(PackedContainer&& container)
{
    return base_observable<utils::extract_observable_type_t<utils::iterable_value_t<std::decay_t<PackedContainer>>>,
                           concat_strategy<std::decay_t<PackedContainer>>>{std::forward<PackedContainer>(container)};
}
}
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