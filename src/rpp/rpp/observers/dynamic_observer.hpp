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

#include <rpp/observers/constraints.hpp>    // wrapping constructor
#include <rpp/observers/state_observer.hpp> // base
#include <rpp/utils/function_traits.hpp>    // extract function args
#include <rpp/utils/functors.hpp>           // default arguments
#include <rpp/defs.hpp>


#include <memory>

namespace rpp::details
{
template<constraint::decayed_type T>
struct dynamic_observer_state_base
{
    virtual ~dynamic_observer_state_base() = default;

    virtual void on_next(const T& v) const = 0;
    virtual void on_next(T&& v) const = 0;
    virtual void on_error(const std::exception_ptr& err) const = 0;
    virtual void on_completed() const = 0;
};

template<constraint::decayed_type T, constraint::observer_of_type<T> TObserver>
class dynamic_observer_state final : public dynamic_observer_state_base<T>
{
public:
    template<typename ...Args>
        requires std::constructible_from<TObserver, Args...>
    dynamic_observer_state(Args&& ...args)
        : m_observer{ std::forward<Args>(args)... } {}

    void on_next(const T& v) const override                     { m_observer.on_next(v);            }
    void on_next(T&& v) const override                          { m_observer.on_next(std::move(v)); }
    void on_error(const std::exception_ptr& err) const override { m_observer.on_error(err);         }
    void on_completed() const override                          { m_observer.on_completed();        }

private:
    RPP_NO_UNIQUE_ADDRESS TObserver m_observer;
};

template<constraint::decayed_type T, constraint::observer_of_type<T> TObserver, typename ...Args>
std::shared_ptr<dynamic_observer_state_base<T>> make_dynamic_observer_state(Args&& ...args) requires std::constructible_from<std::decay_t<TObserver>, Args...>
{
    return std::make_shared<dynamic_observer_state<T, std::decay_t<TObserver>>>(std::forward<Args>(args)...);
}
} // namespace rpp::details

namespace rpp
{
/**
 * \brief Dynamic (type-erased) version of observer (comparing to specific_observer)
 * \details It uses type-erasure mechanism to hide types of OnNext, OnError and OnCompleted callbacks. But it has higher cost in the terms of performance due to usage of heap.
 * Use it only when you need to store observer as member variable or make copy of original subscriber. In other cases prefer using "auto" to avoid converting to dynamic_observer
 * \tparam T is type of value handled by this observer
 * \ingroup observers
 */
template<constraint::decayed_type T>
class dynamic_observer final : public details::typed_observer_tag<T>
{
public:
    template<typename ...States,
        std::invocable<T, std::decay_t<States>...> OnNext,
        std::invocable<std::exception_ptr, std::decay_t<States>...> OnError,
        std::invocable<std::decay_t<States>...> OnCompleted>
    dynamic_observer(OnNext&&      on_next,
                     OnError&&     on_error,
                     OnCompleted&& on_completed,
                     States&& ...  states)
        : m_state{details::make_dynamic_observer_state<T, 
                                                       details::state_observer<T, 
                                                                               std::decay_t<OnNext>, 
                                                                               std::decay_t<OnError>, 
                                                                               std::decay_t<OnCompleted>, 
                                                                               std::decay_t<States>...>>(
                                                                   std::forward<OnNext>(on_next),
                                                                   std::forward<OnError>(on_error),
                                                                   std::forward<OnCompleted>(on_completed),
                                                                   std::forward<States>(states)...)} {}

    template<constraint::on_next_fn<T> OnNext = utils::empty_function_t<T>,
        constraint::on_error_fn OnError = utils::rethrow_error_t,
        constraint::on_completed_fn OnCompleted = utils::empty_function_t<>>
    dynamic_observer(OnNext&& on_next = {}, OnError&& on_error = {}, OnCompleted&& on_completed = {})
        : m_state{details::make_dynamic_observer_state<T, 
                                                       details::state_observer<T, 
                                                                               std::decay_t<OnNext>, 
                                                                               std::decay_t<OnError>, 
                                                                               std::decay_t<OnCompleted>>>(
                                                                   std::forward<OnNext>(on_next),
                                                                   std::forward<OnError>(on_error),
                                                                   std::forward<OnCompleted>(on_completed))} {}

    dynamic_observer(constraint::on_next_fn<T> auto&& on_next, constraint::on_completed_fn auto&& on_completed)
        : dynamic_observer<T>{std::forward<decltype(on_next)>(on_next),
                              utils::rethrow_error_t{},
                              std::forward<decltype(on_completed)>(on_completed)} {}

    dynamic_observer(std::shared_ptr<details::dynamic_observer_state_base<T>> state)
        : m_state{std::move(state)} {}

    template<constraint::observer_of_type<T> TObserver>
        requires (!std::is_same_v<std::decay_t<TObserver>, dynamic_observer<T>>)
    dynamic_observer(TObserver&& obs)
        : m_state{details::make_dynamic_observer_state<T, std::decay_t<TObserver>>(std::forward<TObserver>(obs))} {}


    /**
     * \brief Observable calls this methods to notify observer about new value.
     *
     * \note obtains value by const-reference to original object.
     */
    void on_next(const T& v) const
    {
        m_state->on_next(v);
    }

    /**
     * \brief Observable calls this methods to notify observer about new value.
     *
     * \note obtains value by rvalue-reference to original object
     */
    void on_next(T&& v) const
    {
        m_state->on_next(std::move(v));
    }

    /**
     * \brief Observable calls this method to notify observer about some error during generation next data.
     * \warning Obtaining this call means no any further on_next or on_completed calls
     * \param err details of error
     */
    void on_error(const std::exception_ptr& err) const
    {
        m_state->on_error(err);
    }

    /**
     * \brief Observable calls this method to notify observer about finish of work.
     * \warning Obtaining this call means no any further on_next calls
     */
    void on_completed() const
    {
        m_state->on_completed();
    }

    /**
     * \brief Do nothing for rpp::dynamic_observer. Created only for unification of interfaces with rpp::specific_observer
     */
    const dynamic_observer<T>& as_dynamic() const { return *this; }

private:
    std::shared_ptr<details::dynamic_observer_state_base<T>> m_state{};
};

template<constraint::observer TObserver>
dynamic_observer(TObserver)->dynamic_observer<utils::extract_observer_type_t<TObserver>>;

template<typename OnNext, typename ...Args>
dynamic_observer(OnNext, Args...)->dynamic_observer<utils::decayed_function_argument_t<OnNext>>;
} // namespace rpp
