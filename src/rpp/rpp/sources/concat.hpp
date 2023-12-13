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

#include <rpp/memory_model.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/observables/observable.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/sources/from.hpp>

#include <exception>

namespace rpp::details
{
template<rpp::constraint::observer TObserver, constraint::decayed_type PackedContainer>
struct concat_state_t : public rpp::composite_disposable
    , public std::enable_shared_from_this<concat_state_t<TObserver, PackedContainer>>
{
    concat_state_t(TObserver&& in_observer, const PackedContainer& in_container)
        : observer(std::move(in_observer))
        , container(in_container)
    {
        try
        {
            itr = std::cbegin(container);
        }
        catch(...)
        {
            this->observer.on_error(std::current_exception());
            this->dispose();
        }
    }

    RPP_NO_UNIQUE_ADDRESS TObserver                 observer;
    RPP_NO_UNIQUE_ADDRESS PackedContainer           container;
    std::optional<decltype(std::cbegin(container))> itr{};
    std::atomic<bool>                               is_inside_drain{};
};

template<rpp::constraint::observer TObserver, typename PackedContainer>
void drain(TObserver&& observer, PackedContainer&& container, size_t index);

template<rpp::constraint::observer TObserver, constraint::decayed_type PackedContainer>
struct concat_source_observer_strategy
{
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

    std::shared_ptr<concat_state_t<TObserver, PackedContainer>> state{};
    mutable bool                                                locally_disposed{};

    template<typename T>
    void on_next(T&& v) const
    {
        state->observer.on_next(std::forward<T>(v));
    }

    void on_error(const std::exception_ptr& err) const
    {
        locally_disposed = true;
        state->observer.on_error(err);
    }

    void set_upstream(const disposable_wrapper& d) { state->add(d); }

    bool is_disposed() const { return locally_disposed || state->is_disposed(); }

    void on_completed() const
    {
        locally_disposed = true;
        if (state->is_inside_drain.exchange(false, std::memory_order::seq_cst))
            return;

        drain(state);
    }
};

template<rpp::constraint::observer TObserver, typename PackedContainer>
void drain(const std::shared_ptr<concat_state_t<TObserver, PackedContainer>>& state)
{
    while(!state->is_disposed())
    {
        if (state->itr.value() == std::cend(state->container))
        {
            state->observer.on_completed();
            return;
        }

        using value_type = rpp::utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;
        state->clear();
        state->is_inside_drain.store(false, std::memory_order::seq_cst);
        try
        {
            (*(state->itr.value()++)).subscribe(observer<value_type, concat_source_observer_strategy<std::decay_t<TObserver>, std::decay_t<PackedContainer>>>{state});

            if (state->is_inside_drain.exchange(false, std::memory_order::seq_cst))
                return;
        }
        catch(...)
        {
            state->observer.on_error(std::current_exception());
            return;
        }
    }
}

template<constraint::decayed_type PackedContainer>
struct concat_strategy
{
    template<typename... Args>
        requires (!constraint::variadic_decayed_same_as<concat_strategy<PackedContainer>, Args...>)
    concat_strategy(Args&&... args)
        : container{std::forward<Args>(args)...}
    {
    }

    RPP_NO_UNIQUE_ADDRESS PackedContainer container;

    using value_type = rpp::utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;

    template<constraint::observer_strategy<value_type> Strategy>
    void subscribe(observer<value_type, Strategy>&& obs) const
    {
        auto state = std::make_shared<concat_state_t<observer<value_type, Strategy>, PackedContainer>>(std::move(obs), container);
        state->observer.set_upstream(rpp::disposable_wrapper::from_weak(state));
        drain(state);
    }
};

template<typename PackedContainer, typename... Args>
auto make_concat_from_iterable(Args&&... args)
{
    return observable<utils::extract_observable_type_t<utils::iterable_value_t<std::decay_t<PackedContainer>>>, concat_strategy<std::decay_t<PackedContainer>>>{std::forward<Args>(args)...};
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
 * @tparam MemoryModel rpp::memory_model strategy used to handle provided observables
 *
 * @warning #include <rpp/operators/concat.hpp>
 *
 * @par Example
 * @snippet concat.cpp concat_as_source
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/concat.html
 */
template<constraint::memory_model MemoryModel /*= memory_model::use_stack*/, rpp::constraint::observable TObservable, rpp::constraint::observable... TObservables>
    requires (std::same_as<rpp::utils::extract_observable_type_t<TObservable>, rpp::utils::extract_observable_type_t<TObservables>> && ...)
auto concat(TObservable&& obs, TObservables&&... others)
{
    if constexpr ((rpp::constraint::decayed_same_as<TObservable, TObservables> && ...))
    {
        using inner_container = std::array<std::decay_t<TObservable>, sizeof...(TObservables) + 1>;
        using container       = std::conditional_t<std::same_as<MemoryModel, rpp::memory_model::use_stack>, inner_container, details::shared_container<inner_container>>;
        return rpp::details::make_concat_from_iterable<container>(std::forward<TObservable>(obs), std::forward<TObservables>(others)...);
    }
    else
        return concat<MemoryModel>(std::forward<TObservable>(obs).as_dynamic(), std::forward<TObservables>(others).as_dynamic()...);
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
 * @param iterable is container with observables to subscribe on
 *
 * @tparam MemoryModel rpp::memory_model strategy used to handle provided observables
 * @warning #include <rpp/operators/concat.hpp>
 *
 * @par Example
 * @snippet concat.cpp concat_as_source_vector
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/concat.html
 */
template<constraint::memory_model MemoryModel /*= memory_model::use_stack*/, constraint::iterable Iterable>
    requires constraint::observable<utils::iterable_value_t<Iterable>>
auto concat(Iterable&& iterable)
{
    using Container = std::conditional_t<std::same_as<MemoryModel, rpp::memory_model::use_stack>, std::decay_t<Iterable>, details::shared_container<std::decay_t<Iterable>>>;
    return rpp::details::make_concat_from_iterable<Container>(std::forward<Iterable>(iterable));
}
}
